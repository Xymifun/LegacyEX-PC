#include "game/client/render/TextureUtil.hpp"

#include <algorithm>
#include <cstring>
#include <stb_image.h>
#include <zlib.h>

#include "game/core/GlStateManager.hpp"
#include "game/core/IntBuffer.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/StbiFile.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

class IntBuffer;
class Minecraft;
class StbiFile;

int ProcessLoadedTextureData(unsigned char* data, int* width, int* height, int** pixels);

int ProcessLoadedTextureData(unsigned char* data, int* width, int* height, int** pixels)
{
    int result;

    if (data != NULL)
    {
        unsigned int byteCount = *width * *height * sizeof(int);
        result = 0;
        *pixels = (int*)malloc(byteCount);
        memcpy(*pixels, data, byteCount);
        stbi_image_free(data);

        unsigned int* pixel = (unsigned int*)*pixels;
        for (int i = 0; i < *width * *height; ++i)
        {
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
            const unsigned char* rgba = reinterpret_cast<const unsigned char*>(pixel);
            *pixel = (static_cast<unsigned int>(rgba[3]) << 24) | (static_cast<unsigned int>(rgba[0]) << 16)
                     | (static_cast<unsigned int>(rgba[1]) << 8) | rgba[2];
#else
            TextureUtil::ByteFlipRGBAToARGB(*pixel);
#endif
            ++pixel;
        }
    }
    else
    {
        result = -1;
    }

    return result;
}

static IntBuffer* dataBuffer = NULL;

void TextureUtil::ByteFlipRGBAToARGB(unsigned int& colour)
{
    colour = ((colour & 0xff) << 24) | (colour >> 8);
}

int TextureUtil::generateTextureId()
{
    return GlStateManager::genTexture();
}

void TextureUtil::releaseTextureId(int textureId)
{
    GlStateManager::deleteTexture(textureId);
}

void TextureUtil::bind(int textureId)
{
    GlStateManager::bindTexture(textureId);
}

void TextureUtil::prepareImage(int textureId, int mipLevels, int width, int height)
{
    releaseTextureId(textureId);
    bind(textureId);

    for (int level = 0; level <= mipLevels; level++)
    {
    }
}

void TextureUtil::prepareImage(int textureId, int width, int height)
{
    prepareImage(textureId, 0, width, height);
}

int TextureUtil::anaglyph(int colour)
{
    int alpha = (colour >> 24) & 0xff;
    int red = (colour >> 16) & 0xff;
    int green = (colour >> 8) & 0xff;
    int blue = colour & 0xff;
    int anaglyphRed = (red * 30 + green * 59 + blue * 11) / 100;
    int anaglyphGreen = (red * 30 + green * 70) / 100;
    int anaglyphBlue = (red * 30 + blue * 70) / 100;

    return alpha << 24 | anaglyphRed << 16 | anaglyphGreen << 8 | anaglyphBlue;
}

arrayWithLength<int> TextureUtil::anaglyph(arrayWithLength<int> pixels)
{
    arrayWithLength<int> converted(pixels.length, true);

    for (unsigned int i = 0; i < pixels.length; i++)
    {
        converted[i] = anaglyph(pixels[i]);
    }

    return converted;
}

void TextureUtil::copyToBuffer(arrayWithLength<int> pixels, int offset, int length)
{
    arrayWithLength<int> source = pixels;

    if (Minecraft::GetInstance()->mOptions->mAnaglyph3d)
    {
        source = anaglyph(pixels);
    }

    dataBuffer->clear();
    dataBuffer->put(&source, offset, length);
    dataBuffer->position(0)->limit(length);
}

void TextureUtil::uploadImageChunked(int mipLevel, arrayWithLength<int> pixels, int width, int height, int xOffset,
                                     int yOffset, bool blur, bool clamp, bool mipmap)
{
    int rowsPerChunk = 4194304 / width;

    for (int position = 0; position < width * height;)
    {
        int rows = std::min(rowsPerChunk, height - position / width);
        int count = width * rows;

        copyToBuffer(pixels, position, width * rows);

        position += count;
    }
}

void TextureUtil::uploadImage(int textureId, arrayWithLength<int> pixels, int width, int height)
{
    bind(textureId);
    uploadImageChunked(0, pixels, width, height, 0, 0, false, false, false);
}

void* TextureUtil::decodeJpeg(void* data, int length, int* width, int* height)
{
    int imageWidth;
    int imageHeight;
    int components;
    unsigned char* pixels = stbi_load_from_memory((const unsigned char*)data, length, &imageWidth, &imageHeight,
                                                  &components, 4);

    if (pixels != NULL)
    {
        *width = imageWidth;
        *height = imageHeight;

        for (int y = 0; y < imageHeight; y++)
        {
            for (int x = 0; x < imageWidth; x++)
            {
                unsigned char* pixel = pixels + (y * imageWidth + x) * 4;
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
                unsigned int colour = (static_cast<unsigned int>(pixel[0]) << 24)
                                      | (static_cast<unsigned int>(pixel[1]) << 16)
                                      | (static_cast<unsigned int>(pixel[2]) << 8) | pixel[3];
#else
                unsigned int colour = *(unsigned int*)pixel;
#endif

                pixel[0] = (unsigned char)colour;
                pixel[1] = (unsigned char)(colour >> 8);
                pixel[2] = (unsigned char)(colour >> 16);
                pixel[3] = (unsigned char)(colour >> 24);
            }
        }
    }

    return pixels;
}

void TextureUtil::insertZlibAlphaChannelData(void* pixels, int width, int height, void* compressedData,
                                             int compressedLength)
{
    unsigned char* alphaData = new unsigned char[width * height];
    unsigned long alphaLength = width * height;

    if (uncompress(alphaData, &alphaLength, (const unsigned char*)compressedData, compressedLength) == Z_OK)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = y * width + x;
                unsigned char alpha = alphaData[index];
                unsigned char* pixel = (unsigned char*)pixels + index * 4;
                float alphaScale = alpha / 255.0f;
                float red = pixel[1] / alphaScale;
                float green = pixel[2] / alphaScale;
                float blue = pixel[3] / alphaScale;

                float clampedRed = red < 255.0f ? red : 255.0f;
                float clampedGreen = green < 255.0f ? green : 255.0f;
                float clampedBlue = blue < 255.0f ? blue : 255.0f;

                pixel[0] = alpha;
                pixel[1] = (unsigned char)clampedRed;
                pixel[2] = (unsigned char)clampedGreen;
                pixel[3] = (unsigned char)clampedBlue;
            }
        }
    }

    delete[] alphaData;
}

int TextureUtil::LoadTextureData(const char* fileName, int* width, int* height, int** pixels)
{
    static stbi_io_callbacks callbacks;
    StbiFile file;

    callbacks.read = StbiFile::read;
    callbacks.skip = StbiFile::skip;
    callbacks.eof = StbiFile::eof;

    file.mFilePosition = 0;
    file.mFile = CreateFileA(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    file.mFileSize = file.mFile != INVALID_HANDLE_VALUE ? GetFileSize(file.mFile, NULL) : 0;

    if (file.mFile != INVALID_HANDLE_VALUE)
    {
        int components = 0;
        unsigned char* data = stbi_load_from_callbacks(&callbacks, &file, width, height, &components, 4);
        int result = ProcessLoadedTextureData(data, width, height, pixels);

        if (file.mFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(file.mFile);
        }

        return result;
    }

    return -1;
}

int TextureUtil::LoadTextureData(unsigned char* data, unsigned int length, int* width, int* height, int** pixels)
{
    int components = 0;
    unsigned char* imageData = stbi_load_from_memory(data, length, width, height, &components, 4);

    return ProcessLoadedTextureData(imageData, width, height, pixels);
}
