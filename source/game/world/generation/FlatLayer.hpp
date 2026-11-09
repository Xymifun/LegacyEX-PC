#ifndef GAME_WORLD_GENERATION_FLATLAYER_HPP
#define GAME_WORLD_GENERATION_FLATLAYER_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class FlatLayer : public Layer
{
public:
    FlatLayer(int biome);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

protected:
    int biome;
};

#endif
