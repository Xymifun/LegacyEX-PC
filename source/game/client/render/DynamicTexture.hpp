#ifndef GAME_CLIENT_RENDER_DYNAMICTEXTURE_HPP
#define GAME_CLIENT_RENDER_DYNAMICTEXTURE_HPP

#include "game/client/render/AbstractTexture.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BufferedImage;
class ResourceManager;

class DynamicTexture : public AbstractTexture
{
public:
    DynamicTexture(BufferedImage* image);
    DynamicTexture(int width, int height);

    virtual void load(ResourceManager* resourceManager);

    void upload();
    arrayWithLength<int> getPixels();

private:
    void _init(int width, int height);

    arrayWithLength<int> pixels;
    int width;
    int height;
};

#endif
