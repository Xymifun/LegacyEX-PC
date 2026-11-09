#include "game/core/BufferedImage.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/client/render/TextureUtil.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/DLCPack.hpp"
#include <cstdlib>
#include <cstring>
#include <cmath>

extern char* wstringtofilename(const std::wstring& value, char (&result)[256]);

BufferedImage::BufferedImage(int width, int height, int mipLevels)
{
    m_data[0] = (int*)std::malloc(height * width * sizeof(int));
    std::memset(m_data[0], 0, height * width);
    int i;
    for (i = 1; i < 10; ++i)
        m_data[i] = NULL;
    m_width = (unsigned int)width;
    m_height = (unsigned int)height;
    m_mipmapDisabled = 0;
}

BufferedImage::BufferedImage(const std::wstring& name, bool mipmapDisabled, bool useTitleUpdate,
                             const std::wstring& path)
{
    std::wstring base;
    std::wstring baseName = name;
    m_mipmapDisabled = 0;
    base = path;
    if (base.empty())
    {
        if (useTitleUpdate)
        {
            base = L"Common\\res\\TitleUpdate\\";
        }
        else
        {
            base = L"Common/";
        }
    }

    int i;
    for (i = 0; i < 10; ++i)
        m_data[i] = NULL;

    const int levels = mipmapDisabled ? 1 : 10;
    int level;
    for (level = 0; level < levels; ++level)
    {
        std::wstring file;
        std::wstring suffix = L"";
        if (level != 0)
        {
            suffix = L"MipMapLevel" + _toString(level + 1);
        }

        if (mipmapDisabled)
        {
            file = base + L"res" + baseName.substr(0, baseName.length());
        }
        else
        {
            file = base + L"res" + baseName.substr(0, baseName.length() - 4) + suffix + L".png";
        }

        char filename[256];
        char* resolved = wstringtofilename(file, filename);
        int width = 0;
        int height = 0;
        if (TextureUtil::LoadTextureData(resolved, &width, &height, &m_data[level]) != 0)
        {
            if (level == 0)
            {
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load texture %ls", 0xc3,
                                    convStringToWstring(std::string(resolved)).c_str());
            }
            return;
        }
        if (level == 0)
        {
            m_width = (unsigned int)width;
            m_height = (unsigned int)height;
        }
    }
}

BufferedImage::BufferedImage(DLCPack* pack, const std::wstring& name, bool mipmapDisabled)
{
    std::wstring baseName = name;
    int i;
    for (i = 0; i < 10; ++i)
        m_data[i] = NULL;

    unsigned int imageLength = 0;
    const int levels = mipmapDisabled ? 1 : 10;
    int level;
    for (level = 0; level < levels; ++level)
    {
        std::wstring file;
        std::wstring suffix = L"";
        if (level != 0)
        {
            suffix = L"MipMapLevel" + _toString(level + 1);
        }

        if (mipmapDisabled)
        {
            file = L"res" + baseName.substr(0, baseName.length());
        }
        else
        {
            file = L"res" + baseName + suffix;
        }

        if (!pack->doesPackContainFile(DLCManager::eDLCType_All, file, true, true))
        {
            if (level == 0)
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to find texture %ls", 0xf4, file.c_str());
            return;
        }

        DLCFile* imageFile = pack->getFile(DLCManager::eDLCType_All, file, true, true);
        m_mipmapDisabled = imageFile->getParameterAsBool(DLCManager::eDLCParameterType_DisableMipMap);

        unsigned char* imageData = imageFile->getData(imageLength);
        if (imageData == NULL || imageLength == 0)
        {
            if (level == 0)
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load texture %ls", 0x101, file.c_str());
            return;
        }

        int width = 0;
        int height = 0;
        if (TextureUtil::LoadTextureData(imageData, imageLength, &width, &height, &m_data[level]) != 0)
        {
            if (level == 0)
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load texture data %ls", 0x112, file.c_str());
            return;
        }
        if (level == 0)
        {
            m_width = static_cast<unsigned int>(width);
            m_height = static_cast<unsigned int>(height);
        }
    }
}

BufferedImage::BufferedImage(unsigned char* data, unsigned int length)
{
    int width = 0;
    int height = 0;
    m_mipmapDisabled = 0;
    int i;
    for (i = 0; i < 10; ++i)
        m_data[i] = NULL;
    if (TextureUtil::LoadTextureData(data, length, &width, &height, &m_data[0]) == 0)
    {
        m_width = (unsigned int)width;
        m_height = (unsigned int)height;
    }
    else
    {
        gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load texture data", 0x137);
    }
}

BufferedImage::~BufferedImage()
{
    int i;
    for (i = 0; i < 10; ++i)
        std::free(m_data[i]);
}

unsigned int BufferedImage::getWidth()
{
    return m_width;
}
unsigned int BufferedImage::getHeight()
{
    return m_height;
}

void BufferedImage::getRGB(int x, int y, int width, int height, arrayWithLength<int> rgbArray, int offset, int scansize,
                           int mipLevel)
{
    int sourceWidth = (int)m_width >> mipLevel;
    int* destination = rgbArray.data;
    int row;
    for (row = 0; row < height; ++row)
    {
        int column;
        for (column = 0; column < width; ++column)
        {
            destination[offset + column] = m_data[mipLevel][x + column + (y + row) * sourceWidth];
        }
        offset += scansize;
    }
}

int* BufferedImage::getData()
{
    return m_data[0];
}
int* BufferedImage::getData(int mipLevel)
{
    return m_data[mipLevel];
}
void* BufferedImage::getGraphics()
{
    return NULL;
}

BufferedImage* BufferedImage::getSubimage(int x, int y, int width, int height)
{
    BufferedImage* image = new BufferedImage(width, height, 0);
    arrayWithLength<int> pixels(image->m_data[0], width * height);
    getRGB(x, y, width, height, pixels, 0, width, 0);
    unsigned int level = 1;
    while (getData(level) != NULL)
    {
        int levelWidth = width >> level;
        int levelHeight = height >> level;
        int levelX = x >> level;
        int levelY = y >> level;
        int levelCount = levelHeight * levelWidth;
        int* levelData = (int*)std::malloc(levelCount * sizeof(int));
        image->m_data[level] = levelData;
        getRGB(levelX, levelY, levelWidth, levelHeight, arrayWithLength<int>(levelData, levelCount), 0, levelWidth,
               level);
        ++level;
    }
    return image;
}

unsigned char BufferedImage::isMipmapDisabled()
{
    return m_mipmapDisabled;
}

void BufferedImage::preMultiplyAlpha()
{
    int count = (int)(m_width * m_height);
    int* data = m_data[0];
    int i;
    for (i = 0; i < count; ++i)
    {
        unsigned int p = (unsigned int)data[i];
        int a = (int)(p >> 24);
        int r = (int)((p >> 16) & 255u);
        int g = (int)((p >> 8) & 255u);
        int b = (int)(p & 255u);
        r = (int)(((float)r * (float)a) / 255.0f);
        g = (int)(((float)g * (float)a) / 255.0f);
        b = (int)(((float)b * (float)a) / 255.0f);
        data[i] = (r << 16) | (g << 8) | b | (p & 0xff000000u);
    }
}

#ifdef MINECRAFT_PC
void BufferedImage::PcAddRows(unsigned int height)
{
    if (height <= m_height)
        return;
    for (int level = 0; level < 10; ++level)
    {
        if (m_data[level] == NULL)
            continue;
        const size_t width = m_width >> level;
        const size_t oldRows = m_height >> level;
        const size_t newRows = height >> level;
        int* data = (int*)std::realloc(m_data[level], width * newRows * sizeof(int));
        if (data == NULL)
            continue;
        std::memset(data + width * oldRows, 0, width * (newRows - oldRows) * sizeof(int));
        m_data[level] = data;
    }
    m_height = height;
}
#endif
