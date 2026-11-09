#ifndef GAME_CORE_STITCHER_HPP
#define GAME_CORE_STITCHER_HPP

#include <string>

class TextureHolder;

class Stitcher
{
public:
    Stitcher(const std::wstring& name, int maxWidth, int maxHeight, bool forcePowerOfTwo, int maxTileSize);
    void _init(const std::wstring& name, int maxWidth, int maxHeight, bool forcePowerOfTwo, int maxTileSize);
    int getWidth();
    int getHeight();
    void addTexture(TextureHolder* textureHolder);
    void gatherAreas();
    void constructTexture(bool mipmap);
    void expand(TextureHolder* textureHolder);
    void addToStorage(TextureHolder* textureHolder);
    void stitch();
    int smallestFittingMinTexel(int width, int height);
};

#endif
