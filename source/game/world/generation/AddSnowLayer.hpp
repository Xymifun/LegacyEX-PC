#ifndef GAME_WORLD_GENERATION_ADDSNOWLAYER_HPP
#define GAME_WORLD_GENERATION_ADDSNOWLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class AddSnowLayer : public Layer
{
public:
    AddSnowLayer(long long seed, boost::shared_ptr<Layer> parent);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
