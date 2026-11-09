#ifndef GAME_WORLD_GENERATION_ADDMUSHROOMISLANDLAYER_HPP
#define GAME_WORLD_GENERATION_ADDMUSHROOMISLANDLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/Layer.hpp"

class AddMushroomIslandLayer : public Layer
{
public:
    AddMushroomIslandLayer(long long seed, boost::shared_ptr<Layer> parent);

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
