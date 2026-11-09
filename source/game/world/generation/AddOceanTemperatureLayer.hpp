#ifndef GAME_WORLD_GENERATION_ADDOCEANTEMPERATURELAYER_HPP
#define GAME_WORLD_GENERATION_ADDOCEANTEMPERATURELAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class AddOceanTemperatureLayer : public Layer
{
public:
    AddOceanTemperatureLayer(long long seed, boost::shared_ptr<Layer> parent);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
