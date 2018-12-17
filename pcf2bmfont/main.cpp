#include <iostream>
#include <bitset>
#include "PCFFont.h"

#include <set>
#include <map>

#ifdef _WIN32
#  include <Windows.h>
#  include <stringapiset.h>
#else
#  error Support Windows platform only.
#endif

#include <atlas.h>
#include <bmfont.h>
#include <MaxRectsBinPack.h>
#include <xgetopt.h>

bool read_utf8_tailing(char* p, unsigned int& value)
{
	unsigned int ch = (unsigned int)(*(unsigned char*)p);
	if ((ch & 0xC0) != 0x80)
		return false;

	value = ch & 0x3F;
	return true;
}

int read_utf8_leading(char* p, unsigned int& value)
{
	unsigned int ch = (unsigned int)(*(unsigned char*)p);
	if ((ch & 0x80) == 0)
	{
		value = ch;
		return 0;
	}
	
	if ((ch & 0xE0) == 0xC0) // b110xxxxx b10yyyyyy
	{
		value = ch & 0x1F;
		return 1;
	}

	if ((ch & 0xF0) == 0xE0) // b1110xxxx b10yyyyyy b10zzzzzz
	{
		value = ch & 0x0F;
		return 2;
	}

	if ((ch & 0xF8) == 0xF0) // b11110xxx b10yyyyyy b10zzzzzz b10aaaaaa
	{
		value = ch & 0x07;
		return 3;
	}

	// corrupted
	return -1;
}

bool collect_codepoints(const std::string& path, std::set<unsigned int>& codepoints)
{
	FILE* f = ::fopen(path.c_str(), "rb");
	if (!f)
	{
		std::cerr << "Unable to open file: " << path << " for reading." << std::endl;
		return false;
	}

	bool ret = false;
	char* buf = nullptr;
	do
	{
		::fseek(f, 0, SEEK_END);
		long len = ::ftell(f);
		::fseek(f, 0, SEEK_SET);

		buf = new char[len];
		if (!buf)
			break;

		if ((size_t)len != ::fread(buf, 1, (size_t)len, f))
			break;

		char *p = buf;
		// skip bom
		if (!::strncmp(buf, "\xEF\xBB\xBF", 3))
			p += 3;

		bool corrupted = false;
		while (p < buf + len)
		{
			unsigned int values[4] = {0};
			int remaining = read_utf8_leading(p++, values[0]);
			if (remaining < 0)
			{
				corrupted = true;
				break;
			}

			switch (remaining)
			{
			default:
			case 0:
				codepoints.insert(values[0]);
				break;
			case 1:
				if (!read_utf8_tailing(p++, values[1]))
				{
					corrupted = true;
					break;
				}
				codepoints.insert((values[0] << 6) | values[1]);
				break;
			case 2:
				if (!read_utf8_tailing(p++, values[1]))
				{
					corrupted = true;
					break;
				}
				if (!read_utf8_tailing(p++, values[2]))
				{
					corrupted = true;
					break;
				}
				codepoints.insert((values[0] << 12) | (values[1] << 6) | values[2]);
				break;
			case 3:
				if (!read_utf8_tailing(p++, values[1]))
				{
					corrupted = true;
					break;
				}
				if (!read_utf8_tailing(p++, values[2]))
				{
					corrupted = true;
					break;
				}
				if (!read_utf8_tailing(p++, values[3]))
				{
					corrupted = true;
					break;
				}
				codepoints.insert((values[0] << 18) | (values[1] << 12) | (values[2] << 6) | values[3]);
				break;
			}
		}

		if (corrupted)
		{
			std::cerr << "Corrupted UTF-8 content in file: " << path.c_str() << ", offset: " << ((size_t)p - (size_t)buf) << std::endl;
			break;
		}
		else
			ret = true;
	} while (false);

	::fclose(f);
	delete[] buf;
	return ret;
}

void show_help()
{
	::printf(
		"Usage: \n\tpcf2bmfont [-W width] [-H height] [-n image_filename] [-x xml_filename] [-C] -i char_select_file PCF_font_path\n\tpcf2bmfont -h\n\n"
		"pcf2bmfont generates a BMFont file from given PCF font.\n"
		"\'-W\' and \'-H\' control the output atlas image dimensions (default is 1024).\n"
		"\'-n\' specifies the file name of the output atlas image.\n"
		"\'-x\' specifies the file name of the output BMFont file (in XML format).\n"
		"\'-C\' translate unicode to multi-bytes based on the Active Code Page of current OS.\n"
		"\'-i\' a text file in UTF-8 listing all needed chars. [Required]\n"
		"\'-h\' shows this message.\n");
}

int main(int argc, char *argv[])
{
	int opt;
	int atlasW = 1024;
	int atlasH = 1024;
	bool transcode = false;
	std::string output_atlas_name = "output.png";
	std::string output_xml_name = "output.fnt";
	std::string char_select_file;

	while ((opt = xgetopt(argc, argv, "W:H:hn:x:Ci:")) != -1)
	{
		switch (opt)
		{
		case 'W':
			if (0 >= ::sscanf(xoptarg, "%d", &atlasW))
			{
				fprintf(stderr, "Error: \'%s\' is not a number.", xoptarg);
				return 1;
			}
			break;
		case 'H':
			if (0 >= ::sscanf(xoptarg, "%d", &atlasH))
			{
				fprintf(stderr, "Error: \'%s\' is not a number.", xoptarg);
				return 1;
			}
			break;
		case 'x':
			output_xml_name = xoptarg;
			break;;
		case 'n':
			output_atlas_name = xoptarg;
			break;
		case 'i':
			char_select_file = xoptarg;
			break;
		case 'C':
			transcode = true;
			break;
		default:
		case 'h':
			show_help();
			return (opt == 'h') ? 0 : 1;
		}
	}

	if (char_select_file.empty())
	{
		fprintf(stderr, "Error: Missing char selecting file.\n");
		show_help();
		return 1;
	}

	if (xoptind >= argc)
	{
		fprintf(stderr, "Error: Missing PCF file.\n");
		show_help();
		return 1;
	}

	pcf::PCFFont f = pcf::PCFFont::BuildFromFile(argv[xoptind]);
	if (!f.IsValid())
	{
		std::cerr << f.ErrorMessage() << std::endl;
		return 1;
	}
	short glyph_width = f.GetAcceleratorTable().MaxBounds().CharacterWidth;
	size_t glyph_cnt = f.GetBitmapTable().GetGlyphCount();
	std::cout << "Info: Total " << glyph_cnt << " glyphs in PCF." << std::endl;
	//std::cout << f.GetMetricsTable().GetMetricsCount() << " metrics." << std::endl;

	std::set<unsigned int> codepoints;
	if (!collect_codepoints(char_select_file, codepoints))
	{
		std::cerr << "Error: Parsing error on file: " << char_select_file << std::endl;
		return 1;
	}
	std::cout << "Info: " << codepoints.size() << " codepoints collected from char selecting file." << std::endl;

	std::map<unsigned int, unsigned int> valid_codepoints;
	for (auto cp : codepoints)
	{
		// Skip ASCII range.
		//if (cp < 256)
		//	continue;

		unsigned int codepoint = cp;

		if (transcode)
		{
			unsigned char mbbuf[8] = {0};
			// Note: This is the platform dependent part.
#ifdef _WIN32
			int r = ::WideCharToMultiByte(CP_ACP, 0, (LPCWCH)&cp, 1, (LPSTR)mbbuf, 8, NULL, NULL);
			if (r == 0)
			{
				std::cerr << "Failed to convert unicode codepoint: " << cp << " to Big5 (CP_ACP)" << std::endl;
				continue;
			}
#endif
			codepoint = ((unsigned int)mbbuf[0] << 8) | (unsigned int)mbbuf[1];
		}

		assert(codepoint != 0);
		if (codepoint != 0)
		{
			unsigned int gidx = f.GetEncodingTable().GetGlyphIndex(codepoint);
			if (gidx == 0xffffffff)
			{
				std::cerr << "No corresponding glyph index for codepoint: 0x" 
					<< std::hex << codepoint 
					<< ", unicode: 0x" << cp 
					<< std::dec << std::endl;
			}
			else
				valid_codepoints.insert(std::make_pair(cp, gidx));
		}
	}

	std::vector<rbp::RectSize> rectsizes;
	rectsizes.reserve(valid_codepoints.size());
	rectsizes.assign(valid_codepoints.size(), rbp::RectSize{ glyph_width+1, glyph_width+1 });

	std::vector<rbp::Rect> rects;
	rects.reserve(valid_codepoints.size());
	rbp::MaxRectsBinPack mbp(atlasW, atlasH, false);
	mbp.Insert(rectsizes, rects, rbp::MaxRectsBinPack::RectBestShortSideFit);

	bmfm::BMFontDocument font;
	font.InfoData = bmfm::BMFInfoData{ output_xml_name, -1* glyph_width, false, false, "", true, 100, false, false, {0,0,0,0}, {1,1}, 0};
	font.CommonData = bmfm::BMFCommonData{ (unsigned short)glyph_width, (unsigned short)glyph_width,
		(unsigned short)(unsigned int)atlasW, (unsigned short)(unsigned int)atlasH, 
		1, false, bmfm::BMFChannelMode::Glyph, bmfm::BMFChannelMode::One, bmfm::BMFChannelMode::One, bmfm::BMFChannelMode::One };
	font.PageMap.insert(std::make_pair(0, bmfm::BMFPageData{0, output_atlas_name }));

	bmfm::Atlas a((unsigned int)atlasW, (unsigned int)atlasH);
	std::map<unsigned int, bmfm::BMFCharData>& cmap = font.CharMap;
	for (const auto& pair : valid_codepoints)
	{
		assert(rects.empty() == false);
		const rbp::Rect& rect = rects.back();

		const char* glyph_bitmap = f.GetBitmapTable().GetGlyphBuffer(pair.second);
		const pcf::MetricsData& md = f.GetMetricsTable().GetMetricsData(pair.second);
		unsigned int gw = (unsigned short)md.CharacterWidth;
		unsigned int gh = (unsigned short)(md.CharacterAscent + md.CharacterDescent);

		bmfm::Color pixelColor{255, 255, 255, 255}; // white
		for (unsigned int gy = 0; gy < gh; ++gy)
		{
			for (unsigned int gx = 0; gx < gw; ++gx)
			{
				unsigned char b = (unsigned char)glyph_bitmap[gy*4 + (gx/8)];
				unsigned int gi = gx % 8;
				unsigned char bm = 0x1 << (7-gi);
				if (b & bm)
				{
					a.SetPixel(
						rect.x + gx,
						rect.y + gy,
						pixelColor);
				}
			}
		}

		cmap.insert(std::make_pair(pair.first, 
			bmfm::BMFCharData{pair.first, (unsigned short)rect.x, (unsigned short)rect.y, (unsigned short)gw, (unsigned short)gh, 0, 0, (short)gw, 0, 15}));

		rects.pop_back();
	}

	font.SaveToXML(output_xml_name);
	a.SaveToPNG(output_atlas_name);

	return 0;
}
