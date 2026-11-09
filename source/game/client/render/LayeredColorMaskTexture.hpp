#ifndef GAME_CLIENT_RENDER_LAYEREDCOLORMASKTEXTURE_HPP
#define GAME_CLIENT_RENDER_LAYEREDCOLORMASKTEXTURE_HPP

#include <string>
#include <vector>

#include "game/client/render/AbstractTexture.hpp"
#include "game/core/DyeColor.hpp"
#include "game/util/ResourceLocation.hpp"

class ResourceManager;

class LayeredColorMaskTexture : public AbstractTexture
{
public:
    LayeredColorMaskTexture(ResourceLocation* location, std::vector<std::wstring>* layerPaths,
                            const std::vector<const DyeColor*>* colors);

    virtual void load(ResourceManager* resourceManager);

private:
    ResourceLocation* location;
    std::vector<std::wstring> layerPaths;
    std::vector<const DyeColor*> colors;
};

#endif
