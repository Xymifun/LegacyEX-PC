#ifndef GAME_WORLD_GENERATION_ISLANDLAYER_HPP
#define GAME_WORLD_GENERATION_ISLANDLAYER_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class IslandLayer : public Layer
{
public:
    IslandLayer(long long seed);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
