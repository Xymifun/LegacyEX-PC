#ifndef GAME_WORLD_GENERATION_RIVERMIXERLAYER_HPP
#define GAME_WORLD_GENERATION_RIVERMIXERLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/Layer.hpp"

class RiverMixerLayer : public Layer
{
public:
    RiverMixerLayer(long long seed, boost::shared_ptr<Layer> biomeLayer, boost::shared_ptr<Layer> riverLayer);

    virtual void init(long long seed);
    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

private:
    boost::shared_ptr<Layer> biomeLayer;
    boost::shared_ptr<Layer> riverLayer;
};

#endif
