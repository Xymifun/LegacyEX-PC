#ifndef GAME_WORLD_BIOME_BIOMEOVERRIDELAYER_HPP
#define GAME_WORLD_BIOME_BIOMEOVERRIDELAYER_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class BiomeOverrideLayer : public Layer
{
public:
    BiomeOverrideLayer(int size);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

protected:
    unsigned int overrideWidth;
    unsigned int overrideHeight;
    unsigned char* overrideBiomes;
};

#endif
