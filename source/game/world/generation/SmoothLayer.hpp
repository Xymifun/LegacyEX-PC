#ifndef GAME_WORLD_GENERATION_SMOOTHLAYER_HPP
#define GAME_WORLD_GENERATION_SMOOTHLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class SmoothLayer : public Layer
{
public:
    SmoothLayer(long long seed, boost::shared_ptr<Layer> parent);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
