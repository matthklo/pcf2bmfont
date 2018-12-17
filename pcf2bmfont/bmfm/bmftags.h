#pragma once
#include <string>

namespace bmfm
{

struct BMFInfoData
{
	std::string facename;
	short size = 0; // in pt
	bool bold = false;
	bool italic = false;
	std::string charset; // should leave as empty if unicode is true
	bool unicode = false;
	unsigned short stretchH = 0; // percentage to stretch, 100 means no stretch
	bool smooth = false;
	bool aa = false; // anti-alias
	unsigned char padding[4] = {0,0,0,0}; // up,right,down,left
	unsigned char spacing[2] = {0,0}; // horizontal, verticals
	unsigned char outline = 0; // outline thickness
};

enum class BMFChannelMode
{
	Glyph = 0,
	Outline,
	GlyphAndOutline,
	Zero,
	One,
};

struct BMFCommonData
{
	unsigned short lineHeight = 0; // in pixels
	unsigned short base = 0;
	unsigned short scaleW = 0;
	unsigned short scaleH = 0;
	unsigned short pages = 0;
	bool packed = false;
	BMFChannelMode alphaChannel = BMFChannelMode::Glyph;
	BMFChannelMode redChannel = BMFChannelMode::Glyph;
	BMFChannelMode greenChannel = BMFChannelMode::Glyph;
	BMFChannelMode blueChannel = BMFChannelMode::Glyph;
};

struct BMFPageData
{
	unsigned int id = 0;
	std::string filename;
};

struct BMFCharData
{
	unsigned int id = 0;
	unsigned short x = 0;
	unsigned short y = 0;
	unsigned short width = 0;
	unsigned short height = 0;
	short xoffset = 0;
	short yoffset = 0;
	short xadvance = 0;
	unsigned char page = 0;
	unsigned char channel = 0;
};

struct BMFKerningData
{
	unsigned int first = 0;
	unsigned int second = 0;
	short amount = 0;
};

}; // namespace bmfm
