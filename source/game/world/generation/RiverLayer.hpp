#ifndef GAME_WORLD_GENERATION_RIVERLAYER_HPP
#define GAME_WORLD_GENERATION_RIVERLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/Layer.hpp"

class RiverLayer : public Layer
{
public:
    RiverLayer(long long seed, boost::shared_ptr<Layer> parent);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

private:
    int riverFilter(int biomeId);
};

#endif
