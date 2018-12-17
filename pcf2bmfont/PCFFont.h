#pragma once

/*
  Reference PCF spec: https://fontforge.github.io/en-US/documentation/reference/pcf-format/
 */

#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>

namespace pcf
{

class Format final
{
public:
	Format() : mRawFormatValue(0) {}
	Format(int raw_format) : mRawFormatValue(raw_format) {}

	// Gross formats
	bool IsDefaultFormat() const { return (mRawFormatValue & 0xFFFFFF00) == 0; }
	bool IsInkBounds() const { return (mRawFormatValue & 0xFFFFFF00) == 0x200; }
	bool IsAcceleratorWithInkBounds() const { return (mRawFormatValue & 0xFFFFFF00) == 0x100; }
	bool IsCompressedMetrics() const { return (mRawFormatValue & 0xFFFFFF00) == 0x100; }

	// Modifiers
	bool IsMostSigByteFirst() const { return (mRawFormatValue & 0x4) != 0; }
	bool IsMostSigBitFirst() const { return (mRawFormatValue & 0x8) != 0; }
	int GetGlyphPadding() const { return mRawFormatValue & 0x3; }
	int GetScanUnits() const { return (mRawFormatValue >> 4) & 0x3; }

private:
	int mRawFormatValue;
};

class PropertiesTable final
{
public:
	void BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }

	const Format& GetFormat() const { return mFormat; }
	const std::unordered_map<std::string, int>& GetIntegerPropertiesMap() const { return mIntegerProperties; }
	const std::unordered_map<std::string, std::string>& GetStringPropertiesMap() const { return mStringProperties; }

private:
	bool mIsValid = false;

	Format mFormat;
	std::unordered_map<std::string, int> mIntegerProperties;
	std::unordered_map<std::string, std::string> mStringProperties;
};

struct MetricsData final
{
	static size_t GetUncompressedLength() { return 12; }
	static size_t GetCompressedLength() { return 5; }

	void BuildFromData(const char* buf, bool is_compressed, bool big_endian);

	bool WasCompressed = false;
	short LeftSideBearing = 0;
	short RightSideBearing = 0;
	short CharacterWidth = 0;
	short CharacterAscent = 0;
	short CharacterDescent = 0;
	unsigned short CharacterAttributes = 0;
};

class AcceleratorTable final
{
public:
	void BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }

	const Format& GetFormat() const { return mFormat; }
	bool NoOverlap() const { return mNoOverlap; }
	bool ConstantMetrics() const { return mConstantMetrics; }
	bool TerminalFont() const { return mTerminalFont; }
	bool ConstantWidth() const { return mConstantWidth; }
	bool InkInside() const { return mInkInside; }
	bool InkMetrics() const { return mInkMetrics; }
	unsigned char DrawDirection() const { return mDrawDirection; }
	int FontAscent() const { return mFontAscent; }
	int FontDescent() const { return mFontDescent; }
	int MaxOverlap() const { return mMaxOverlap; }

	const MetricsData& MinBounds() const { return mMinBounds; }
	const MetricsData& MaxBounds() const { return mMaxBounds; }

	// Meaningful if only GetFormat().IsAcceleratorWithInkBounds() == true:
	const MetricsData& InkMinBounds() const { return mInkMinBounds; }
	const MetricsData& InkMaxBounds() const { return mInkMaxBounds; }

private:
	bool mIsValid = false;

	Format mFormat;
	bool mNoOverlap = false;
	bool mConstantMetrics = false;
	bool mTerminalFont = false;
	bool mConstantWidth = false;
	bool mInkInside = false;
	bool mInkMetrics = false; /* true if the ink metrics differ from the metrics somewhere */
	unsigned char mDrawDirection = 0; // 0: left to right, 1: right to left
	int mFontAscent = 0;
	int mFontDescent = 0;
	int mMaxOverlap = 0;

	MetricsData mMinBounds;
	MetricsData mMaxBounds;
	
	// Meaningful if only mFormat.IsAcceleratorWithInkBounds() == true:
	MetricsData mInkMinBounds;
	MetricsData mInkMaxBounds;
};

class MetricsTable final
{
public:
	void BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }

	const Format& GetFormat() const { return mFormat; }
	bool IsCompressedMetrics() const { return mFormat.IsCompressedMetrics(); }
	size_t GetMetricsCount() const { return mMetrics.size(); }
	const MetricsData& GetMetricsData(unsigned int index) const { return mMetrics[index]; }

private:
	bool mIsValid = false;

	Format mFormat;
	std::vector<MetricsData> mMetrics;
};

class BitmapTable final
{
public:
	void BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }

	const Format& GetFormat() const { return mFormat; }
	size_t GetGlyphCount() const { return mGlyphDataOffsets.size(); }
	const char* GetGlyphBuffer(unsigned int index) const {
		return &mRawGylphBuffer[mGlyphDataOffsets[index]]; }

private:
	bool mIsValid = false;

	Format mFormat;
	std::vector<unsigned int> mGlyphDataOffsets;
	std::vector<char> mRawGylphBuffer;
};

class EncodingTable final
{
public:
	void BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }

	const Format& GetFormat() const { return mFormat; }
	size_t GetGlyphIndexCount() const
	{
		size_t cnt = (size_t)((mMaxCharOrByte2 - mMinCharOrByte2 + 1)*(mMaxByte1 - mMinByte1 + 1)); 
		assert(cnt == mGlyphIndexes.size());
		return mGlyphIndexes.size();
	}

	unsigned int GetGlyphIndex(unsigned short codepoint) const
	{
		if (((codepoint & 0xFF00) == 0) && (mMinByte1 == 0) && (mMaxByte1 == 0))
		{
			// single byte encodings
			if (codepoint > (unsigned short)mMaxCharOrByte2 || codepoint < (unsigned short)mMinCharOrByte2)
				return 0xffffffff;

			return (unsigned int)mGlyphIndexes[codepoint - (unsigned short)mMinCharOrByte2];
		}

		unsigned short enc1 = (codepoint >> 8) & 0xFF;
		unsigned short enc2 = codepoint & 0xFF;

		if (enc1 > (unsigned short)mMaxByte1 || enc1 < (unsigned short)mMinByte1)
			return 0xffffffff;
		if (enc2 > (unsigned short)mMaxCharOrByte2 || enc2 < (unsigned short)mMinCharOrByte2)
			return 0xffffffff;

		return (unsigned int)mGlyphIndexes[
			(enc1 - (unsigned short)mMinByte1)*(mMaxCharOrByte2- mMinCharOrByte2+1) + (enc2 - mMinCharOrByte2)];
	}

private:
	bool mIsValid = false;

	Format mFormat;
	short mMinCharOrByte2 = 0;
	short mMaxCharOrByte2 = 0;
	short mMinByte1 = 0;
	short mMaxByte1 = 0;
	short mDefaultChar = 0;
	std::vector<short> mGlyphIndexes; /* a value of 0xffff means no glyph for that encoding */
};

class ScalableWidthsTable final
{
public:
	void BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }
	
	const Format& GetFormat() const { return mFormat; }
	size_t GetScalableWidthCount() const { return mScalableWidths.size(); }
	int GetScalableWidth(unsigned int index) const { return mScalableWidths[index]; }

private:
	bool mIsValid = false;
	
	Format mFormat;
	std::vector<int> mScalableWidths;
};

class GlyphNamesTable final
{
public:
	void BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }

	const Format& GetFormat() const { return mFormat; }
	size_t GetGlyphNamesCount() const { return mGlyphNames.size(); }
	const std::string& GetGlyphName(unsigned int index) const
	{ return mGlyphNames[index]; }

private:
	bool mIsValid = false;

	Format mFormat;
	std::vector<std::string> mGlyphNames; /* should be the same size as metrics */
};

class PCFFont final
{
public:
	static PCFFont BuildFromFile(const std::string& path);

	bool BuildFromData(const char* buf);
	bool IsValid() const { return mIsValid; }
	const std::string& ErrorMessage() const { return mErrorMessage; }

	const PropertiesTable& GetPropertiesTable() const
	{ return mPropertiesTable; }
	const AcceleratorTable& GetAcceleratorTable() const
	{ return mAcceleratorTable; }
	const AcceleratorTable& GetBDFAccerleratorTable() const
	{ return mBDFAccerleratorTable; }
	const MetricsTable& GetMetricsTable() const
	{ return mMetricsTable; }
	const MetricsTable& GetInkMetricsTable() const
	{ return mInkMetricsTable; }
	const BitmapTable& GetBitmapTable() const
	{ return mBitmapTable; }
	const ScalableWidthsTable& GetScalableWidthsTable() const
	{ return mScalableWidthsTable; }
	const GlyphNamesTable& GetGlyphNamesTable() const
	{ return mGlyphNamesTable; }
	const EncodingTable GetEncodingTable() const
	{ return mEncodingTable; }

private:
	bool mIsValid = false;
	std::string mErrorMessage;

	PropertiesTable mPropertiesTable;
	AcceleratorTable mAcceleratorTable;
	AcceleratorTable mBDFAccerleratorTable;
	MetricsTable mMetricsTable;
	MetricsTable mInkMetricsTable;
	BitmapTable mBitmapTable;
	ScalableWidthsTable mScalableWidthsTable;
	GlyphNamesTable mGlyphNamesTable;
	EncodingTable mEncodingTable;
};

};
