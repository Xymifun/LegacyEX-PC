#ifndef GAME_CLIENT_RENDER_TEXTUREUTIL_HPP
#define GAME_CLIENT_RENDER_TEXTUREUTIL_HPP

#include <string>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/array2DWithLength.hpp"

class BufferedImage;
class InputStream;
class ResourceManager;
class ResourceLocation;

class TextureUtil
{
public:
    static void staticCtor();

    static int generateTextureId();
    static void releaseTextureId(int textureId);
    static void bind(int textureId);

    static void setFilter(bool blur, bool mipmap);
    static void setFilter(bool blur);
    static void setClamp(bool clamp);

    static void prepareImage(int textureId, int mipLevels, int width, int height);
    static void prepareImage(int textureId, int width, int height);

    static int anaglyph(int colour);
    static arrayWithLength<int> anaglyph(arrayWithLength<int> pixels);

    static void copyToBuffer(arrayWithLength<int> pixels, int length);
    static void copyToBuffer(arrayWithLength<int> pixels, int offset, int length);

    static void uploadImage(int textureId, arrayWithLength<int> pixels, int width, int height);
    static void uploadImage(int textureId, BufferedImage* image, int xOffset, int yOffset, bool blur, bool clamp);
    static void uploadImage(int textureId, BufferedImage* image, bool blur, bool clamp);
    static void uploadImage(int textureId, BufferedImage* image);

    static void uploadImageChunked(int mipLevel, arrayWithLength<int> pixels, int width, int height, int xOffset,
                                   int yOffset, bool blur, bool clamp, bool mipmap);
    static void uploadImageChunked(BufferedImage* image, int xOffset, int yOffset, bool blur, bool clamp);
    static void uploadImageChunked(array2DWithLength<int> mipPixels, int width, int height, int xOffset, int yOffset,
                                   bool blur, bool clamp);

    static array2DWithLength<int> generateMipLevels(int mipLevels, int width, array2DWithLength<int> mipPixels);

    static int alphaBlend(int topLeft, int topRight);
    static int alphaBlend(int topLeft, int topRight, int bottomLeft, int bottomRight);
    static int alphaBlend(int topLeft, int topRight, int bottomLeft, int bottomRight, bool skipTransparent);
    static int gammaBlend(int topLeft, int topRight, int bottomLeft, int bottomRight, int shift);

    static arrayWithLength<int> getPixels(ResourceManager* resourceManager, const ResourceLocation& location);
    static BufferedImage* readImage(InputStream* stream);
    static void writeAsPNG(const std::wstring& fileName, int textureId, int mipLevels, int width, int height);

    static arrayWithLength<int> flipVertical(arrayWithLength<int> pixels, int width, int height);

    static void* decodeJpeg(void* data, int length, int* width, int* height);
    static void insertZlibAlphaChannelData(void* pixels, int width, int height, void* compressedData,
                                           int compressedLength);

    static int LoadTextureData(const char* fileName, int* width, int* height, int** pixels);
    static int LoadTextureData(unsigned char* data, unsigned int length, int* width, int* height, int** pixels);
    static void ByteFlipRGBAToARGB(unsigned int& colour);
    static void ByteFlipRGBAToBGRA(unsigned int& colour);
    static void ByteFlipARGBToABGR(unsigned int& colour);
    static void ByteFlipARGBToRGBA(unsigned int& colour);
};

#endif
