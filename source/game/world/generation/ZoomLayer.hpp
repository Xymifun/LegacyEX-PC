#ifndef GAME_WORLD_GENERATION_ZOOMLAYER_HPP
#define GAME_WORLD_GENERATION_ZOOMLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class ZoomLayer : public Layer
{
public:
    static boost::shared_ptr<Layer> zoom(long long seed, boost::shared_ptr<Layer> parent, int count);

    ZoomLayer(long long seed, boost::shared_ptr<Layer> parent);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
