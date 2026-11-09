#ifndef GAME_WORLD_GENERATION_IMPOSECONTINENTSLAYER_HPP
#define GAME_WORLD_GENERATION_IMPOSECONTINENTSLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class ImposeContinentsLayer : public Layer
{
public:
    ImposeContinentsLayer(long long seed, boost::shared_ptr<Layer> parent);
    virtual ~ImposeContinentsLayer();

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
