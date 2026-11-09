#ifndef GAME_WORLD_GENERATION_REMOVETOOMUCHOCEANLAYER_HPP
#define GAME_WORLD_GENERATION_REMOVETOOMUCHOCEANLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class RemoveTooMuchOceanLayer : public Layer
{
public:
    RemoveTooMuchOceanLayer(long long seed, boost::shared_ptr<Layer> parent);
    virtual ~RemoveTooMuchOceanLayer();

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
