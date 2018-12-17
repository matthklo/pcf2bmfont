#include "PCFFont.h"

using namespace pcf;

struct props_data
{
	int name_offset = 0;
	char isStringProp = 0;
	int value = 0;
};

struct toc_entry
{
	int type = 0;
	int format = 0;
	int size = 0;
	int offset = 0;
};

enum class endian_type
{
	little = 0,
	big,
};

static endian_type _host_endian()
{
	unsigned short v = 0xABCD;
	if (*((char*)&v) == 0xAB)
		return endian_type::big;
	return endian_type::little;
}

static int _read_int(const char* buf, endian_type endian)
{
	if (endian == _host_endian())
		return *(int*)buf;

	// swap endianness
	char buf_swapped[4] = {buf[3], buf[2], buf[1], buf[0]};
	return *(int*)&buf_swapped;
}

static short _read_short(const char* buf, endian_type endian)
{
	if (endian == _host_endian())
		return *(short*)buf;

	// swap endianness
	char buf_swapped[2] = {buf[1], buf[0]};
	return *(short*)&buf_swapped;
}

void PropertiesTable::BuildFromData(const char* buf)
{
	mIsValid = false;
	mIntegerProperties.clear();
	mStringProperties.clear();

	mFormat = Format(*(int*)buf); buf += sizeof(int);
	endian_type endian = mFormat.IsMostSigByteFirst() ? endian_type::big : endian_type::little;

	int prop_cnt = _read_int(buf, endian); buf+=sizeof(int);
	std::vector<props_data> props_data_array;
	props_data_array.reserve(prop_cnt);
	for (int i=0; i<prop_cnt; ++i)
	{
		props_data d;
		d.name_offset = _read_int(buf, endian); buf += sizeof(int);
		d.isStringProp = *buf; buf++;
		d.value = _read_int(buf, endian); buf += sizeof(int);
		props_data_array.push_back(d);
	}

	//skip padding
	buf += ((prop_cnt & 3) == 0 ? 0 : (4 - (prop_cnt & 3)));

	const char* strbuf_base = (buf + sizeof(int));
	for (size_t i=0; i < props_data_array.size(); ++i)
	{
		const props_data& d = props_data_array[i];
		if (d.isStringProp)
			mStringProperties.insert(std::make_pair(
				std::string(&strbuf_base[d.name_offset]),
				std::string(&strbuf_base[d.value])));
		else
			mIntegerProperties.insert(std::make_pair(
				std::string(&strbuf_base[d.name_offset]),
				d.value));
	}

	mIsValid = true;
}

void MetricsData::BuildFromData(const char* buf, bool is_compressed, bool big_endian)
{
	WasCompressed = is_compressed;
	if (is_compressed)
	{
		unsigned char v = *(unsigned char*)buf; buf++;
		LeftSideBearing = (short)(v - 0x80);
		v = *(unsigned char*)buf; buf++;
		RightSideBearing = (short)(v - 0x80);
		v = *(unsigned char*)buf; buf++;
		CharacterWidth = (short)(v - 0x80);
		v = *(unsigned char*)buf; buf++;
		CharacterAscent = (short)(v - 0x80);
		v = *(unsigned char*)buf;
		CharacterDescent = (short)(v - 0x80);
		CharacterAttributes = 0;
		return;
	}

	endian_type endian = big_endian ? endian_type::big : endian_type::little;

	LeftSideBearing = _read_short(buf, endian); buf += sizeof(short);
	RightSideBearing = _read_short(buf, endian); buf += sizeof(short);
	CharacterWidth = _read_short(buf, endian); buf += sizeof(short);
	CharacterAscent = _read_short(buf, endian); buf += sizeof(short);
	CharacterDescent = _read_short(buf, endian); buf += sizeof(short);
	CharacterAttributes = (unsigned short)_read_short(buf, endian);
}

void AcceleratorTable::BuildFromData(const char* buf)
{
	mIsValid = false;

	mFormat = Format(*(int*)buf); buf += sizeof(int);
	endian_type endian = mFormat.IsMostSigByteFirst() ? endian_type::big : endian_type::little;

	mNoOverlap = ((*(char*)buf) != 0); buf++;
	mConstantMetrics = ((*(char*)buf) != 0); buf++;
	mTerminalFont = ((*(char*)buf) != 0); buf++;
	mConstantWidth = ((*(char*)buf) != 0); buf++;
	mInkInside = ((*(char*)buf) != 0); buf++;
	mInkMetrics = ((*(char*)buf) != 0); buf++;
	mDrawDirection = ((*(unsigned char*)buf) != 0); buf+=2; /* skip a padding byte as well */
	mFontAscent = _read_int(buf, endian); buf += sizeof(int);
	mFontDescent = _read_int(buf, endian); buf += sizeof(int);
	mMaxOverlap = _read_int(buf, endian); buf += sizeof(int);
	mMinBounds.BuildFromData(buf, false, mFormat.IsMostSigByteFirst());
	assert(mMinBounds.WasCompressed == false);
	buf+= MetricsData::GetUncompressedLength();
	mMaxBounds.BuildFromData(buf, false, mFormat.IsMostSigByteFirst());
	assert(mMaxBounds.WasCompressed == false);
	buf += MetricsData::GetUncompressedLength();

	if (mFormat.IsAcceleratorWithInkBounds())
	{
		mInkMinBounds.BuildFromData(buf, false, mFormat.IsMostSigByteFirst());
		assert(mInkMinBounds.WasCompressed == false);
		buf += MetricsData::GetUncompressedLength();
		mInkMaxBounds.BuildFromData(buf,false, mFormat.IsMostSigByteFirst());
		assert(mInkMaxBounds.WasCompressed == false);
	}
	else
	{
		mInkMinBounds = mMinBounds;
		mInkMaxBounds = mMaxBounds;
	}

	mIsValid = true;
}

void MetricsTable::BuildFromData(const char* buf)
{
	mIsValid = false;
	mMetrics.clear();

	mFormat = Format(*(int*)buf); buf += sizeof(int);
	endian_type endian = mFormat.IsMostSigByteFirst() ? endian_type::big : endian_type::little;

	if (mFormat.IsCompressedMetrics())
	{
		short cnt = _read_short(buf, endian); buf += sizeof(short);
		mMetrics.reserve((size_t)cnt);
		for (short i=0; i<cnt; ++i)
		{
			MetricsData d;
			d.BuildFromData(buf, true, mFormat.IsMostSigByteFirst());
			mMetrics.push_back(d);
			buf += MetricsData::GetCompressedLength();
		}
	}
	else
	{
		int cnt = _read_int(buf, endian); buf += sizeof(int);
		mMetrics.reserve((size_t)cnt);
		for (int i = 0; i < cnt; ++i)
		{
			MetricsData d;
			d.BuildFromData(buf, false, mFormat.IsMostSigByteFirst());
			mMetrics.push_back(d);
			buf += MetricsData::GetUncompressedLength();
		}
	}

	mIsValid = true;
}

void BitmapTable::BuildFromData(const char* buf)
{
	mIsValid = false;
	mGlyphDataOffsets.clear();
	mRawGylphBuffer.clear();

	mFormat = Format(*(int*)buf); buf += sizeof(int);
	endian_type endian = mFormat.IsMostSigByteFirst() ? endian_type::big : endian_type::little;

	int glyph_cnt = _read_int(buf, endian); buf += sizeof(int);
	mGlyphDataOffsets.reserve(glyph_cnt);
	for (int i=0; i<glyph_cnt; ++i)
	{
		mGlyphDataOffsets.push_back(_read_int(buf, endian));
		buf += sizeof(int);
	}
	
	int bitmap_sizes[4];
	bitmap_sizes[0] = _read_int(buf, endian); buf += sizeof(int);
	bitmap_sizes[1] = _read_int(buf, endian); buf += sizeof(int);
	bitmap_sizes[2] = _read_int(buf, endian); buf += sizeof(int);
	bitmap_sizes[3] = _read_int(buf, endian); buf += sizeof(int);

	int bitmap_size = bitmap_sizes[mFormat.GetGlyphPadding()];
	mRawGylphBuffer.reserve(bitmap_size);
	mRawGylphBuffer.assign(bitmap_size, 0);
	::memcpy(&mRawGylphBuffer[0], buf, bitmap_size);

	mIsValid = true;
}

void EncodingTable::BuildFromData(const char* buf)
{
	mIsValid = false;
	mGlyphIndexes.clear();

	mFormat = Format(*(int*)buf); buf += sizeof(int);
	endian_type endian = mFormat.IsMostSigByteFirst() ? endian_type::big : endian_type::little;

	mMinCharOrByte2 = _read_short(buf, endian); buf += sizeof(short);
	mMaxCharOrByte2 = _read_short(buf, endian); buf += sizeof(short);
	mMinByte1 = _read_short(buf, endian); buf += sizeof(short);
	mMaxByte1 = _read_short(buf, endian); buf += sizeof(short);
	mDefaultChar = _read_short(buf, endian); buf += sizeof(short);

	unsigned int cnt = 
		((unsigned int)mMaxCharOrByte2 - (unsigned int)mMinCharOrByte2 + 1) *
		((unsigned int)mMaxByte1 - (unsigned int)mMinByte1 + 1);
	mGlyphIndexes.reserve(cnt);
	for (unsigned int i=0; i<cnt; ++i)
	{
		mGlyphIndexes.push_back(_read_short(buf, endian));
		buf += sizeof(short);
	}

	mIsValid = true;
}

void ScalableWidthsTable::BuildFromData(const char *buf)
{
	mIsValid = false;
	mScalableWidths.clear();

	mFormat = Format(*(int*)buf); buf += sizeof(int);
	endian_type endian = mFormat.IsMostSigByteFirst() ? endian_type::big : endian_type::little;

	int cnt = _read_int(buf, endian); buf+=sizeof(int);
	mScalableWidths.reserve(cnt);
	for (int i=0; i<cnt; ++i)
	{
		mScalableWidths.push_back(_read_int(buf, endian));
		buf += sizeof(int);
	}

	mIsValid = true;
}

void GlyphNamesTable::BuildFromData(const char* buf)
{
	mIsValid = false;
	mGlyphNames.clear();

	mFormat = Format(*(int*)buf); buf += sizeof(int);
	endian_type endian = mFormat.IsMostSigByteFirst() ? endian_type::big : endian_type::little;

	std::vector<int> offsets;
	int cnt = _read_int(buf, endian); buf += sizeof(int);
	offsets.reserve(cnt);
	for (int i=0; i<cnt; ++i)
	{
		offsets.push_back(_read_int(buf, endian));
		buf += sizeof(int);
	}

	buf += sizeof(int);

	mGlyphNames.reserve(cnt);
	for (int i=0; i<cnt; ++i)
	{
		mGlyphNames.emplace_back(std::string(&buf[offsets[i]]));
	}

	mIsValid = true;
}

PCFFont PCFFont::BuildFromFile(const std::string& path)
{
	PCFFont ret;

	FILE* f = nullptr;
	char* file_buf = nullptr;
	do
	{
		f = ::fopen(path.c_str(), "rb");
		if (!f)
		{
			ret.mErrorMessage = std::string("Unable to open file: ") + path.c_str();
			break;
		}

		::fseek(f, 0, SEEK_END);
		long len = ::ftell(f);
		::fseek(f, 0, SEEK_SET);

		file_buf = new char[len];
		if ((size_t)len != ::fread(file_buf, 1, len, f))
		{
			ret.mErrorMessage = std::string("Failed on mmap for file: ") + path.c_str();
			break;
		}

		ret.BuildFromData(file_buf);
	} while (false);

	if (nullptr != f)
		::fclose(f);
	delete[] file_buf;
	return ret;
}

#define PCF_PROPERTIES       (1<<0)
#define PCF_ACCELERATORS     (1<<1)
#define PCF_METRICS          (1<<2)
#define PCF_BITMAPS          (1<<3)
#define PCF_INK_METRICS      (1<<4)
#define PCF_BDF_ENCODINGS    (1<<5)
#define PCF_SWIDTHS          (1<<6)
#define PCF_GLYPH_NAMES      (1<<7)
#define PCF_BDF_ACCELERATORS (1<<8)

bool PCFFont::BuildFromData(const char* buf)
{
	if (0 != ::strncmp(buf, "\1fcp", 4))
	{
		mErrorMessage = std::string("Invalid PCF magic.");
		return false;
	}

	const char* p = buf + 4;
	int toc_cnt = *(int*)p; p += sizeof(int);
	std::vector<toc_entry> entries;
	entries.reserve(toc_cnt);
	for (int i=0; i<toc_cnt; ++i)
	{
		toc_entry e;
		e.type = _read_int(p, endian_type::little); p += sizeof(int);
		e.format = _read_int(p, endian_type::little); p += sizeof(int);
		e.size = _read_int(p, endian_type::little); p += sizeof(int);
		e.offset = _read_int(p, endian_type::little); p += sizeof(int);
		entries.push_back(e);
	}

	for (size_t i=0; i<entries.size(); ++i)
	{
		const toc_entry& e = entries[i];
		switch (e.type)
		{
		case PCF_PROPERTIES:
			mPropertiesTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_ACCELERATORS:
			mAcceleratorTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_METRICS:
			mMetricsTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_BITMAPS:
			mBitmapTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_INK_METRICS:
			mInkMetricsTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_BDF_ENCODINGS:
			mEncodingTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_SWIDTHS:
			mScalableWidthsTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_GLYPH_NAMES:
			mGlyphNamesTable.BuildFromData(&buf[e.offset]);
			break;
		case PCF_BDF_ACCELERATORS:
			mBDFAccerleratorTable.BuildFromData(&buf[e.offset]);
			break;
		default:
			mErrorMessage = std::string("Unexpected type value in an toc entry: ") + std::to_string(e.type);
			return false;
		}
	}

	mIsValid = true;
	return true;
}
