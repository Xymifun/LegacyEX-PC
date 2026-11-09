#include "game/client/render/LayeredColorMaskTexture.hpp"

LayeredColorMaskTexture::LayeredColorMaskTexture(ResourceLocation* location, std::vector<std::wstring>* layerPaths,
                                                 const std::vector<const DyeColor*>* colors)
    : location(location),
      layerPaths(*layerPaths),
      colors(*colors)
{
}

void LayeredColorMaskTexture::load(ResourceManager* resourceManager)
{
}
