#ifdef USE_VLD
#  include <vld.h>
#endif

#include "atlas.h"
#include "utils.h"
#include <cassert>
#include <png.h>
#include <string.h>
#include <stdlib.h>

using namespace bmfm;

static void _png_error_callback(::png_structp structp, const char* msg)
{
	bmfm::logerrfmt("libpng error: file: %s, msg: %s",
		(const char*)png_get_error_ptr(structp), msg);
}

static void _png_warn_callback(::png_structp structp, const char* msg)
{
	bmfm::logerrfmt("libpng warning: file: %s, msg: %s",
		(const char*)png_get_error_ptr(structp), msg);
}


Atlas::Atlas()
	: mWidth(0)
	, mHeight(0)
	, mBuffer(nullptr)
{
}

Atlas::Atlas(unsigned int w, unsigned int h)
	: mWidth(w)
	, mHeight(h)
{
	size_t len = (size_t)w*(size_t)h * 4;
	mBuffer = new char[len];
	::memset(mBuffer, 0, len);
}

Atlas::~Atlas()
{
	Free();
}

Atlas::Atlas(const Atlas& that)
{
	mBuffer = nullptr;
	*this = that;
}

Atlas::Atlas(Atlas&& that)
{
	mBuffer = nullptr;
	*this = that;
}

Atlas& Atlas::operator=(const Atlas& that)
{
	Free();

	// clone
	mPath = that.mPath;
	mWidth = that.mWidth;
	mHeight = that.mHeight;
	if (that.mBuffer == nullptr)
		mBuffer = nullptr;
	else
	{
		size_t len = (size_t)mWidth*(size_t)mHeight * 4;
		mBuffer = new char[len];
		::memcpy(mBuffer, that.mBuffer, len);
	}
	return *this;
}

Atlas& Atlas::operator=(Atlas&& that)
{
	Free();

	// move
	mPath = that.mPath; that.mPath = "";
	mWidth = that.mWidth; that.mWidth = 0;
	mHeight = that.mHeight; that.mHeight = 0;
	mBuffer = that.mBuffer; that.mBuffer = nullptr;
	return *this;
}

Atlas Atlas::LoadFromPNG(std::string path)
{
	Atlas ret;
	ret.mPath = path;

	FILE *f = ::fopen(path.c_str(), "rb");
	if (!f)
	{
		logerrfmt("Atlas::LoadFromPNG(): Failed to open file for reading. %s", path.c_str());
		::abort();
	}

	::png_structp png_ptr = nullptr;
	::png_infop info_ptr = nullptr;
	::png_infop end_ptr = nullptr;

	unsigned char pngheader[8];
	::fread(pngheader, 1, 8, f);
	if (0 != ::png_sig_cmp(pngheader, 0, 8))
	{
		logerrfmt("Atlas::LoadFromPNG(): %s is not a valid PNG file.", path.c_str());
		::abort();
	}

	png_ptr = ::png_create_read_struct(PNG_LIBPNG_VER_STRING, (void*)path.c_str(), _png_error_callback, _png_warn_callback);
	if (!png_ptr)
	{
		logerrfmt("Atlas::LoadFromPNG(): png_create_read_struct() failed. %s", path.c_str());
		::abort();
	}

	info_ptr = ::png_create_info_struct(png_ptr);
	end_ptr = ::png_create_info_struct(png_ptr);
	if (!info_ptr || !end_ptr)
	{
		logerrfmt("Atlas::LoadFromPNG(): png_create_info_struct() failed. %s", path.c_str());
		::abort();
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		logerrfmt("Atlas::LoadFromPNG(): png_init_io() failed. %s", path.c_str());
		::abort();
	}

	::png_init_io(png_ptr, f);
	::png_set_sig_bytes(png_ptr, 8);
	::png_read_info(png_ptr, info_ptr);
		
	unsigned int width = ::png_get_image_width(png_ptr, info_ptr);
	unsigned int height = ::png_get_image_height(png_ptr, info_ptr);
	unsigned char color_type = ::png_get_color_type(png_ptr, info_ptr);
	unsigned char bit_depth = ::png_get_bit_depth(png_ptr, info_ptr);
	size_t rowbytes = ::png_get_rowbytes(png_ptr, info_ptr);

	// FIXME: For now expect only RGBA, 8bits per channel images.
	if (color_type != PNG_COLOR_TYPE_RGBA || bit_depth != 8)
	{
		logerrfmt("Error: Unexpected color_type:%u (should be %u) or bit_depth:%u (should be 8).\nMake sure the input PNG is in RGBA 32bits format.",
			color_type, PNG_COLOR_TYPE_RGBA, bit_depth);
		::abort();
	}

	assert(rowbytes == width * 4);

	ret.mWidth = width;
	ret.mHeight = height;
	ret.mBuffer = new char[width*height*4];

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		logerrfmt("Atlas::LoadFromPNG(): png_read_image() failed. %s", path.c_str());
		::abort();
	}

	png_bytep* rawbuf = new png_bytep[height];
	for (unsigned int i=0; i<height; ++i)
		rawbuf[i] = (unsigned char*)&ret.mBuffer[i*rowbytes];
	::png_read_image(png_ptr, rawbuf);
	delete[] rawbuf;

	::png_destroy_read_struct(&png_ptr, &info_ptr, &end_ptr);
	::fclose(f);
	return ret;
}

bool Atlas::SaveToPNG(std::string path)
{
	if (mWidth == 0 || mHeight == 0 || mBuffer == nullptr)
	{
		logerr("Error: Atlas::SaveToPNG(): Unable to save an empty atlas.");
		return false;
	}

	FILE* f = ::fopen(path.c_str(), "wb");
	if (!f)
	{
		logerrfmt("Error: Atlas::SaveToPNG(): Unable to open file: %s for writing.", path.c_str());
		return false;
	}

	::png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;

	png_ptr = ::png_create_write_struct(PNG_LIBPNG_VER_STRING, (void*)path.c_str(), _png_error_callback, _png_warn_callback);
	if (!png_ptr)
	{
		logerr("Error: Atlas::SaveToPNG(): png_create_write_struct() failed.");
		::fclose(f);
		return false;
	}

	info_ptr = ::png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		logerr("Error: Atlas::SaveToPNG(): png_create_info_struct() failed.");
		::png_destroy_write_struct(&png_ptr, &info_ptr);
		::fclose(f);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		logerr("Error: Atlas::SaveToPNG(): png_init_io() failed.");
		::png_destroy_write_struct(&png_ptr, &info_ptr);
		::fclose(f);
		return false;
	}

	::png_init_io(png_ptr, f);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		logerr("Error: Atlas::SaveToPNG(): Failed on writing PNG header.");
		::png_destroy_write_struct(&png_ptr, &info_ptr);
		::fclose(f);
		return false;
	}

	::png_set_IHDR(png_ptr, info_ptr, mWidth, mHeight,
		8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	::png_write_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		logerr("Error: Atlas::SaveToPNG(): png_write_image() failed.");
		::png_destroy_write_struct(&png_ptr, &info_ptr);
		::fclose(f);
		return false;
	}

	png_bytep* rawbuf = new png_bytep[mHeight];
	for (unsigned int i = 0; i < mHeight; ++i)
		rawbuf[i] = (unsigned char*)&mBuffer[i*mWidth*4];
	::png_write_image(png_ptr, rawbuf);
	delete[] rawbuf;

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		logerr("Error: Atlas::SaveToPNG(): png_write_end() failed.");
		::png_destroy_write_struct(&png_ptr, &info_ptr);
		::fclose(f);
		return false;
	}

	::png_write_end(png_ptr, nullptr);
	::png_destroy_write_struct(&png_ptr, &info_ptr);
	::fclose(f);
	return true;
}

void Atlas::Free()
{
	delete[] mBuffer;
	mBuffer = nullptr;
	mHeight = mWidth = 0;
	mPath = "";
}

bool Atlas::BitBlt(const Atlas& srcAtlas,
	unsigned int dstX, unsigned int dstY,
	unsigned int srcX, unsigned int srcY,
	unsigned int width, unsigned int height)
{
	if (dstX >= mWidth || (dstX + width) >= mWidth ||
		dstY >= mHeight || (dstY + height) >= mHeight ||
		srcX >= srcAtlas.GetWidth() || (srcX + width) >= srcAtlas.GetWidth() ||
		srcY >= srcAtlas.GetHeight() || (srcY + height) >= srcAtlas.GetHeight())
	{
		logerr("Warning: Atlas::BitBlt(): Either dst or src rect is out of range.");
		return false;
	}

	for (unsigned int y = 0; y < height; ++y)
	{
		unsigned int sy = srcY + y;
		unsigned int dy = dstY + y;

		unsigned int src_row_stride = srcAtlas.GetWidth() * 4;
		unsigned int dst_row_stride = mWidth * 4;

		::memcpy(&mBuffer[(dy*dst_row_stride) + (dstX * 4)],
			&srcAtlas.mBuffer[(sy*src_row_stride) + (srcX * 4)],
			width*4);
	}

	return true;
}

void Atlas::SetPixel(unsigned int x, unsigned int y, const Color& c)
{
	if (x >= mWidth || y >= mHeight || mBuffer == nullptr)
	{
		logerr("Warning: Atlas::SetPixel(): Either x or y is out of range.");
		return;
	}

	unsigned int row_stride = mWidth*4;
	char* p = &mBuffer[(y*row_stride) + (x*4)];
	p[0] = (char)c.R;
	p[1] = (char)c.G;
	p[2] = (char)c.B;
	p[3] = (char)c.A;
}

Color Atlas::GetPixel(unsigned int x, unsigned int y) const
{
	Color ret = {0};

	if (x >= mWidth || y >= mHeight || mBuffer == nullptr)
	{
		logerr("Warning: Atlas::GetPixel(): Either x or y is out of range.");
		return ret;
	}

	unsigned int row_stride = mWidth * 4;
	char* p = &mBuffer[(y*row_stride) + (x * 4)];
	ret.R = (unsigned char)p[0];
	ret.G = (unsigned char)p[1];
	ret.B = (unsigned char)p[2];
	ret.A = (unsigned char)p[3];
	return ret;
}
