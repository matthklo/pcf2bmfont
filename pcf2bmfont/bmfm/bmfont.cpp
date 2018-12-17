#ifdef USE_VLD
#  include <vld.h>
#endif

#include "bmfont.h"
#include "utils.h"
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <list>

using namespace bmfm;

BMFontDocument::BMFontDocument()
{
}

BMFontDocument BMFontDocument::LoadFromXML(std::string path)
{
	// Try open the file at given path. mmap the content.
	char* file_content = mmap(path.c_str());
	
	if (!file_content)
	{
		logerrfmt("BMFontDocument::LoadFromXML(): Failed on mmap() for file %s", path.c_str());
		::abort();
	}

	BMFontDocument ret;
	do
	{
		rapidxml::xml_document<> doc;
		doc.parse<0>(file_content); // throw exception on error

		rapidxml::xml_node<>* fontnode = doc.first_node("font");
		if (nullptr == fontnode)
		{
			logerrfmt("BMFontDocument::LoadFromXML(): Missing <font> tag in XML. %s", path.c_str());
			::abort();
		}

#define NAG(_TAG,_ATTR) \
	logerrfmt("BMFontDocument::LoadFromXML(): Invalid \"" _ATTR "\" value in the \"" _TAG "\" tag. (%s)", path.c_str());\
	::abort();

		rapidxml::xml_node<>* n = fontnode->first_node();
		do
		{
			std::string tagname = n->name();
			if (tagname == "info")
			{
				for (rapidxml::xml_attribute<> *attr = n->first_attribute();
					attr; attr = attr->next_attribute())
				{
					std::string attrname = attr->name();
					if (attrname == "face")
					{
						ret.InfoData.facename = attr->value();
					}
					else if (attrname == "size")
					{
						int value = 0;
						if (0 >= ::sscanf(attr->value(), "%d", &value))
						{
							NAG("info","size");
						}
						ret.InfoData.size = (short)value;
					}
					else if (attrname == "bold")
					{
						int value = 0;
						if (0 >= ::sscanf(attr->value(), "%d", &value))
						{
							NAG("info", "bold");
						}
						ret.InfoData.bold = (value != 0);
					}
					else if (attrname == "italic")
					{
						int value = 0;
						if (0 >= ::sscanf(attr->value(), "%d", &value))
						{
							NAG("info", "italic");
						}
						ret.InfoData.italic = (value != 0);
					}
					else if (attrname == "charset")
					{
						ret.InfoData.charset = attr->value();
					}
					else if (attrname == "unicode")
					{
						int value = 0;
						if (0 >= ::sscanf(attr->value(), "%d", &value))
						{
							NAG("info", "unicode");
						}
						ret.InfoData.unicode = (value != 0);
					}
					else if (attrname == "stretchH")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("info", "stretchH");
						}
						ret.InfoData.stretchH = (unsigned short)value;
					}
					else if (attrname == "smooth")
					{
						int value = 0;
						if (0 >= ::sscanf(attr->value(), "%d", &value))
						{
							NAG("info", "smooth");
						}
						ret.InfoData.smooth = (value != 0);
					}
					else if (attrname == "aa")
					{
						int value = 0;
						if (0 >= ::sscanf(attr->value(), "%d", &value))
						{
							NAG("info", "aa");
						}
						ret.InfoData.aa = (value != 0);
					}
					else if (attrname == "padding")
					{
						unsigned int value[4] = {0};
						if (0 >= ::sscanf(attr->value(), "%u,%u,%u,%u", &value[0], &value[1], &value[2], &value[3]))
						{
							NAG("info", "padding");
						}
						ret.InfoData.padding[0] = (unsigned char)value[0];
						ret.InfoData.padding[1] = (unsigned char)value[1];
						ret.InfoData.padding[2] = (unsigned char)value[2];
						ret.InfoData.padding[3] = (unsigned char)value[3];
					}
					else if (attrname == "spacing")
					{
						unsigned int value[2] = {0};
						if (0 >= ::sscanf(attr->value(), "%u,%u", &value[0], &value[1]))
						{
							NAG("info", "spacing");
						}
						ret.InfoData.spacing[0] = (unsigned char)value[0];
						ret.InfoData.spacing[1] = (unsigned char)value[1];
					}
					else if (attrname == "outline")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("info", "outline");
						}
						ret.InfoData.outline = (unsigned char)value;
					}
				}
			}
			else if (tagname == "common")
			{
				for (rapidxml::xml_attribute<> *attr = n->first_attribute();
					attr; attr = attr->next_attribute())
				{
					std::string attrname = attr->name();
					if (attrname == "lineHeight")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "lineHeight");
						}
						ret.CommonData.lineHeight = (unsigned short)value;
					}
					else if (attrname == "base")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "base");
						}
						ret.CommonData.base = (unsigned short)value;
					}
					else if (attrname == "scaleW")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "scaleW");
						}
						ret.CommonData.scaleW = (unsigned short)value;
					}
					else if (attrname == "scaleH")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "scaleH");
						}
						ret.CommonData.scaleH = (unsigned short)value;
					}
					else if (attrname == "pages")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "pages");
						}
						ret.CommonData.pages = (unsigned short)value;
					}
					else if (attrname == "packed")
					{
						int value = 0;
						if (0 >= ::sscanf(attr->value(), "%d", &value))
						{
							NAG("common", "packed");
						}
						ret.CommonData.packed = (value != 0);
					}
					else if (attrname == "alphaChnl")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "alphaChnl");
						}
						if (value > 4) value = 4;
						ret.CommonData.alphaChannel = (BMFChannelMode)value;
					}
					else if (attrname == "redChnl")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "redChnl");
						}
						if (value > 4) value = 4;
						ret.CommonData.redChannel = (BMFChannelMode)value;
					}
					else if (attrname == "greenChnl")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "greenChnl");
						}
						if (value > 4) value = 4;
						ret.CommonData.greenChannel = (BMFChannelMode)value;
					}
					else if (attrname == "blueChnl")
					{
						unsigned int value = 0;
						if (0 >= ::sscanf(attr->value(), "%u", &value))
						{
							NAG("common", "blueChnl");
						}
						if (value > 4) value = 4;
						ret.CommonData.blueChannel = (BMFChannelMode)value;
					}
				}
			}
			else if (tagname == "pages")
			{
				for (rapidxml::xml_node<>* pagenode = n->first_node();
					pagenode; pagenode = pagenode->next_sibling())
				{
					std::string nodename = pagenode->name();
					if (nodename != "page") continue;

					BMFPageData pd;
					for (rapidxml::xml_attribute<> *pattr = pagenode->first_attribute();
						pattr; pattr = pattr->next_attribute())
					{
						std::string pattrname = pattr->name();
						if (pattrname == "id")
						{
							if (0 >= ::sscanf(pattr->value(), "%u", &pd.id))
							{
								NAG("page", "id");
							}
						}
						else if (pattrname == "file")
						{
							pd.filename = pattr->value();
						}
					}

					Atlas a = Atlas::LoadFromPNG(pd.filename);
					ret.AltasMap.insert(std::make_pair(pd.id, a));
					ret.PageMap.insert(std::make_pair(pd.id, pd));
				}
			}
			else if (tagname == "chars")
			{
				for (rapidxml::xml_node<>* charnode = n->first_node();
					charnode; charnode = charnode->next_sibling())
				{
					std::string nodename = charnode->name();
					if (nodename != "char") continue;

					BMFCharData cd;
					for (rapidxml::xml_attribute<> *cattr = charnode->first_attribute();
						cattr; cattr = cattr->next_attribute())
					{
						std::string cattrname = cattr->name();
						if (cattrname == "id")
						{
							if (0 >= ::sscanf(cattr->value(), "%u", &cd.id))
							{
								NAG("char", "id");
							}
						}
						else if (cattrname == "x")
						{
							unsigned int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%u", &value))
							{
								NAG("char", "x");
							}
							cd.x = (unsigned short)value;
						}
						else if (cattrname == "y")
						{
							unsigned int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%u", &value))
							{
								NAG("char", "y");
							}
							cd.y = (unsigned short)value;
						}
						else if (cattrname == "width")
						{
							unsigned int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%u", &value))
							{
								NAG("char", "width");
							}
							cd.width = (unsigned short)value;
						}
						else if (cattrname == "height")
						{
							unsigned int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%u", &value))
							{
								NAG("char", "height");
							}
							cd.height = (unsigned short)value;
						}
						else if (cattrname == "xoffset")
						{
							int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%d", &value))
							{
								NAG("char", "xoffset");
							}
							cd.xoffset = (short)value;
						}
						else if (cattrname == "yoffset")
						{
							int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%d", &value))
							{
								NAG("char", "yoffset");
							}
							cd.yoffset = (short)value;
						}
						else if (cattrname == "xadvance")
						{
							int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%d", &value))
							{
								NAG("char", "xadvance");
							}
							cd.xadvance = (short)value;
						}
						else if (cattrname == "page")
						{
							unsigned int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%u", &value))
							{
								NAG("char", "page");
							}
							cd.page = (unsigned char)value;
						}
						else if (cattrname == "chnl")
						{
							unsigned int value = 0;
							if (0 >= ::sscanf(cattr->value(), "%u", &value))
							{
								NAG("char", "chnl");
							}
							cd.channel = (unsigned char)value;
						}
					}
					ret.CharMap.insert(std::make_pair(cd.id, cd));
				}
			}
			else if (tagname == "kernings")
			{
				for (rapidxml::xml_node<>* kernnode = n->first_node();
					kernnode; kernnode = kernnode->next_sibling())
				{
					std::string nodename = kernnode->name();
					if (nodename != "kerning") continue;

					BMFKerningData kd;
					for (rapidxml::xml_attribute<> *kattr = kernnode->first_attribute();
						kattr; kattr = kattr->next_attribute())
					{
						std::string kattrname = kattr->name();
						if (kattrname == "first")
						{
							if (0 >= ::sscanf(kattr->value(), "%u", &kd.first))
							{
								NAG("kerning", "first");
							}
						}
						else if (kattrname == "second")
						{
							if (0 >= ::sscanf(kattr->value(), "%u", &kd.second))
							{
								NAG("kerning", "second");
							}
						}
						else if (kattrname == "amount")
						{
							int value;
							if (0 >= ::sscanf(kattr->value(), "%d", &value))
							{
								NAG("kerning", "amount");
							}
							kd.amount = (short)value;
						}
					}
					unsigned long long id = kd.first;
					id <<= 32;
					id |= kd.second;
					ret.KerningMap.insert(std::make_pair(id, kd));
				}
			}

			n = n->next_sibling();
		} while (n);
	} while(0);

	return ret;
}

bool BMFontDocument::SaveToXML(std::string path)
{
	// Build XML doc
	rapidxml::xml_document<> doc;
	auto* fontnode = doc.allocate_node(rapidxml::node_element, "font");
	doc.append_node(fontnode);

	auto* infonode = doc.allocate_node(rapidxml::node_element, "info");
	fontnode->append_node(infonode);
	auto* faceattr = doc.allocate_attribute("face", InfoData.facename.c_str());
	infonode->append_attribute(faceattr);
	auto sizestr = std::to_string(InfoData.size);
	auto* sizeattr = doc.allocate_attribute("size", sizestr.c_str());
	infonode->append_attribute(sizeattr);
	auto* boldattr = doc.allocate_attribute("bold", InfoData.bold ? "1" : "0");
	infonode->append_attribute(boldattr);
	auto* italicattr = doc.allocate_attribute("italic", InfoData.italic ? "1" : "0");
	infonode->append_attribute(italicattr);
	auto* charsetattr = doc.allocate_attribute("charset", InfoData.charset.c_str());
	infonode->append_attribute(charsetattr);
	auto* unicodeattr = doc.allocate_attribute("unicode", InfoData.unicode ? "1" : "0");
	infonode->append_attribute(unicodeattr);
	auto stretchHstr = std::to_string(InfoData.stretchH);
	auto* stretchHattr = doc.allocate_attribute("stretchH", stretchHstr.c_str());
	infonode->append_attribute(stretchHattr);
	auto* smoothattr = doc.allocate_attribute("smooth", InfoData.smooth ? "1" : "0");
	infonode->append_attribute(smoothattr);
	auto* aaattr = doc.allocate_attribute("aa", InfoData.aa ? "1" : "0");
	infonode->append_attribute(aaattr);
	char padbuf[64];
	::sprintf(padbuf, "%u,%u,%u,%u", InfoData.padding[0], InfoData.padding[1], InfoData.padding[2], InfoData.padding[3]);
	auto* paddingattr = doc.allocate_attribute("padding", padbuf);
	infonode->append_attribute(paddingattr);
	char spacingbuf[64];
	::sprintf(spacingbuf, "%u,%u", InfoData.spacing[0], InfoData.spacing[1]);
	auto* spacingattr = doc.allocate_attribute("spacing", spacingbuf);
	infonode->append_attribute(spacingattr);
	auto outlinestr = std::to_string((unsigned int)InfoData.outline);
	auto* outlineattr = doc.allocate_attribute("outline", outlinestr.c_str());
	infonode->append_attribute(outlineattr);

	auto* commonnode = doc.allocate_node(rapidxml::node_element, "common");
	fontnode->append_node(commonnode);
	auto lineheightstr = std::to_string(CommonData.lineHeight);
	auto* lineheightattr = doc.allocate_attribute("lineHeight", lineheightstr.c_str());
	commonnode->append_attribute(lineheightattr);
	auto basestr = std::to_string(CommonData.base);
	auto* baseattr = doc.allocate_attribute("base", basestr.c_str());
	commonnode->append_attribute(baseattr);
	auto scaleWstr = std::to_string(CommonData.scaleW);
	auto* scaleWattr = doc.allocate_attribute("scaleW", scaleWstr.c_str());
	commonnode->append_attribute(scaleWattr);
	auto scaleHstr = std::to_string(CommonData.scaleH);
	auto* scaleHattr = doc.allocate_attribute("scaleH", scaleHstr.c_str());
	commonnode->append_attribute(scaleHattr);
	auto pagesstr = std::to_string(CommonData.pages);
	auto* pagesattr = doc.allocate_attribute("pages", pagesstr.c_str());
	commonnode->append_attribute(pagesattr);
	auto* packedattr = doc.allocate_attribute("packed", CommonData.packed ? "1" : "0");
	commonnode->append_attribute(packedattr);
	auto achstr = std::to_string((unsigned int)CommonData.alphaChannel);
	auto* achattr = doc.allocate_attribute("alphaChnl", achstr.c_str());
	commonnode->append_attribute(achattr);
	auto rchstr = std::to_string((unsigned int)CommonData.redChannel);
	auto* rchattr = doc.allocate_attribute("redChnl", rchstr.c_str());
	commonnode->append_attribute(rchattr);
	auto gchstr = std::to_string((unsigned int)CommonData.greenChannel);
	auto* gchattr = doc.allocate_attribute("greenChnl", gchstr.c_str());
	commonnode->append_attribute(gchattr);
	auto bchstr = std::to_string((unsigned int)CommonData.blueChannel);
	auto* bchattr = doc.allocate_attribute("blueChnl", bchstr.c_str());
	commonnode->append_attribute(bchattr);

	std::list<std::string> sh;

	auto* pagesnode = doc.allocate_node(rapidxml::node_element, "pages");
	fontnode->append_node(pagesnode);
	for (auto it = PageMap.begin(); it != PageMap.end(); ++it)
	{
		const BMFPageData& pd = it->second;
		auto* pagenode = doc.allocate_node(rapidxml::node_element, "page");
		pagesnode->append_node(pagenode);
		sh.emplace_back(std::to_string(pd.id));
		auto* idattr = doc.allocate_attribute("id", sh.back().c_str());
		pagenode->append_attribute(idattr);
		auto* fileattr = doc.allocate_attribute("file", pd.filename.c_str());
		pagenode->append_attribute(fileattr);
	}

	if (!CharMap.empty())
	{
		auto* charsnode = doc.allocate_node(rapidxml::node_element, "chars");
		fontnode->append_node(charsnode);
		sh.emplace_back(std::to_string(CharMap.size()));
		auto* ccntattr = doc.allocate_attribute("count", sh.back().c_str());
		charsnode->append_attribute(ccntattr);
		for (auto it = CharMap.begin(); it != CharMap.end(); ++it)
		{
			const BMFCharData& cd = it->second;
			auto* charnode = doc.allocate_node(rapidxml::node_element, "char");
			charsnode->append_node(charnode);
			sh.emplace_back(std::to_string(cd.id));
			auto* idattr = doc.allocate_attribute("id", sh.back().c_str());
			charnode->append_attribute(idattr);
			sh.emplace_back(std::to_string(cd.x));
			auto* xattr = doc.allocate_attribute("x", sh.back().c_str());
			charnode->append_attribute(xattr);
			sh.emplace_back(std::to_string(cd.y));
			auto* yattr = doc.allocate_attribute("y", sh.back().c_str());
			charnode->append_attribute(yattr);
			sh.emplace_back(std::to_string(cd.width));
			auto* widthattr = doc.allocate_attribute("width", sh.back().c_str());
			charnode->append_attribute(widthattr);
			sh.emplace_back(std::to_string(cd.height));
			auto* heightattr = doc.allocate_attribute("height", sh.back().c_str());
			charnode->append_attribute(heightattr);
			sh.emplace_back(std::to_string(cd.xoffset));
			auto* xoffsetattr = doc.allocate_attribute("xoffset", sh.back().c_str());
			charnode->append_attribute(xoffsetattr);
			sh.emplace_back(std::to_string(cd.yoffset));
			auto* yoffsetattr = doc.allocate_attribute("yoffset", sh.back().c_str());
			charnode->append_attribute(yoffsetattr);
			sh.emplace_back(std::to_string(cd.xadvance));
			auto* xadvanceattr = doc.allocate_attribute("xadvance", sh.back().c_str());
			charnode->append_attribute(xadvanceattr);
			sh.emplace_back(std::to_string((unsigned int)cd.page));
			auto* pageattr = doc.allocate_attribute("page", sh.back().c_str());
			charnode->append_attribute(pageattr);
			sh.emplace_back(std::to_string((unsigned int)cd.channel));
			auto* chattr = doc.allocate_attribute("chnl", sh.back().c_str());
			charnode->append_attribute(chattr);
		}
	}

	if (!KerningMap.empty())
	{
		auto* kerningsnode = doc.allocate_node(rapidxml::node_element, "kernings");
		fontnode->append_node(kerningsnode);
		sh.emplace_back(std::to_string(KerningMap.size()));
		auto* kcntattr = doc.allocate_attribute("count", sh.back().c_str());
		kerningsnode->append_attribute(kcntattr);
		for (auto it = KerningMap.begin(); it != KerningMap.end(); ++it)
		{
			const BMFKerningData& kd = it->second;
			auto* kernnode = doc.allocate_node(rapidxml::node_element, "kerning");
			kerningsnode->append_node(kernnode);
			sh.emplace_back(std::to_string(kd.first));
			auto* firstattr = doc.allocate_attribute("first", sh.back().c_str());
			kernnode->append_attribute(firstattr);
			sh.emplace_back(std::to_string(kd.second));
			auto* secondattr = doc.allocate_attribute("second", sh.back().c_str());
			kernnode->append_attribute(secondattr);
			sh.emplace_back(std::to_string(kd.amount));
			auto* amountattr = doc.allocate_attribute("amount", sh.back().c_str());
			kernnode->append_attribute(amountattr);
		}
	}

	// print out XML to a std::string
	std::string xmlbuf;
	rapidxml::print(std::back_inserter(xmlbuf), doc);

	// Write to file
	FILE* f = ::fopen(path.c_str(), "wb");
	if (!f)
	{
		logerrfmt("SaveToXML(): Failed on opening file %s for writing.", path.c_str());
		return false;
	}

	// Write XML header
	char hdrbuf[64] = {0};
	::sprintf(hdrbuf, "%s", "<?xml version=\"1.0\"?>\n");
	::fwrite(hdrbuf, 1, ::strlen(hdrbuf), f);

	// Write XML content
	::fwrite(xmlbuf.c_str(), 1, xmlbuf.size(), f);
	::fclose(f);
	return true;
}
