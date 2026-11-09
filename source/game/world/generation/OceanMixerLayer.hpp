#ifndef GAME_WORLD_GENERATION_OCEANMIXERLAYER_HPP
#define GAME_WORLD_GENERATION_OCEANMIXERLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class OceanMixerLayer : public Layer
{
public:
    OceanMixerLayer(long long seed, boost::shared_ptr<Layer> landLayer, boost::shared_ptr<Layer> oceanLayer);

    virtual void init(long long seed);
    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

private:
    boost::shared_ptr<Layer> landLayer;
    boost::shared_ptr<Layer> oceanLayer;
};

#endif
