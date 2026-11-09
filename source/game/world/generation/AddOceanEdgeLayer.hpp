#ifndef GAME_WORLD_GENERATION_ADDOCEANEDGELAYER_HPP
#define GAME_WORLD_GENERATION_ADDOCEANEDGELAYER_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class AddOceanEdgeLayer : public Layer
{
public:
    AddOceanEdgeLayer(long long seed, boost::shared_ptr<Layer> parent, bool useOceanTemperatures);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
