#include "game/client/render/DynamicTexture.hpp"

#include "game/client/render/TextureUtil.hpp"
void DynamicTexture::_init(int width, int height)
{
    this->width = width;
    this->height = height;

    pixels = arrayWithLength<int>(width * height, true);

    TextureUtil::prepareImage(getId(), width, height);
}

void DynamicTexture::upload()
{
    TextureUtil::uploadImage(getId(), pixels, width, height);
}

void DynamicTexture::load(ResourceManager* resourceManager)
{
}
