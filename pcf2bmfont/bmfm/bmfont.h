#pragma once

#include <string>
#include <map>
#include "bmftags.h"
#include "atlas.h"

namespace bmfm
{

class BMFontDocument
{
public:
	BMFontDocument();
	BMFontDocument(const BMFontDocument& that) = default;
	BMFontDocument(BMFontDocument&& that) = default;

	BMFontDocument& operator=(const BMFontDocument& that) = default;
	BMFontDocument& operator=(BMFontDocument&& that) = default;

public:
	static BMFontDocument LoadFromXML(std::string path);
	bool SaveToXML(std::string path);

public:
	BMFInfoData InfoData;
	BMFCommonData CommonData;
	std::map<unsigned int, BMFPageData> PageMap;
	std::map<unsigned int, Atlas> AltasMap;
	std::map<unsigned int, BMFCharData> CharMap;
	std::map<unsigned long long, BMFKerningData> KerningMap;

private:

}; // class BMFontDocument

}; // namespace bmfm
