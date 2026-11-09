#ifndef GAME_CORE_BUFFEREDIMAGE_HPP
#define GAME_CORE_BUFFEREDIMAGE_HPP

#include <string>
#include "game/world/biome/arrayWithLength.hpp"

class DLCPack;

class BufferedImage
{
private:
    int* m_data[10];
    unsigned int m_width;
    unsigned int m_height;
    unsigned char m_mipmapDisabled;

public:
    BufferedImage(int width, int height, int mipLevels);
    BufferedImage(const std::wstring& name, bool mipmapDisabled, bool useTitleUpdate, const std::wstring& path);
    BufferedImage(DLCPack* pack, const std::wstring& name, bool mipmapDisabled);
    BufferedImage(unsigned char* data, unsigned int length);
    ~BufferedImage();

    unsigned int getWidth();
    unsigned int getHeight();
    void getRGB(int x, int y, int width, int height, arrayWithLength<int> rgbArray, int offset, int scansize,
                int mipLevel);
    int* getData();
    int* getData(int mipLevel);
    void* getGraphics();
    BufferedImage* getSubimage(int x, int y, int width, int height);
    unsigned char isMipmapDisabled();
    void preMultiplyAlpha();
    static void ByteFlip4(unsigned int& value);
    int getTransparency();
#ifdef MINECRAFT_PC
    void PcAddRows(unsigned int height);
#endif
};

#endif
