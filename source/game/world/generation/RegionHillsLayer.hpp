#ifndef GAME_WORLD_GENERATION_REGIONHILLSLAYER_HPP
#define GAME_WORLD_GENERATION_REGIONHILLSLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class RegionHillsLayer : public Layer
{
public:
    RegionHillsLayer(long long seed, boost::shared_ptr<Layer> parent, boost::shared_ptr<Layer> riverLayer);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

    boost::shared_ptr<Layer> riverLayer;
};

#endif
