#pragma once
#include <string>

namespace bmfm
{

struct Color
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
	unsigned char A;
};

class Atlas
{
public:
	Atlas();
	Atlas(unsigned int w, unsigned int h);
	virtual ~Atlas();
	Atlas(const Atlas& that);
	Atlas(Atlas&& that);
	Atlas& operator=(const Atlas& that);
	Atlas& operator=(Atlas&& that);

	static Atlas LoadFromPNG(std::string path);
	bool SaveToPNG(std::string path);

	bool BitBlt(const Atlas& srcAtlas, 
		unsigned int dstX, unsigned int dstY, 
		unsigned int srcX, unsigned int srcY,
		unsigned int width, unsigned int height);

	unsigned int GetWidth() const { return mWidth; }
	unsigned int GetHeight() const { return mHeight; }
	const std::string& GetPath() const { return mPath; }
	void SetPixel(unsigned int x, unsigned int y, const Color& c);
	Color GetPixel(unsigned int x, unsigned int y) const;

private:
	void Free();

	unsigned int mWidth;
	unsigned int mHeight;
	std::string mPath;
	char* mBuffer; // Buffer pointing to a pixel buffer. (RGBA)
};

}; // namespace bmfm
