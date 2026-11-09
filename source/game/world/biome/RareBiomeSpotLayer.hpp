#ifndef GAME_WORLD_BIOME_RAREBIOMESPOTLAYER_HPP
#define GAME_WORLD_BIOME_RAREBIOMESPOTLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Layer.hpp"

class RareBiomeSpotLayer : public Layer
{
public:
    RareBiomeSpotLayer(long long seed, boost::shared_ptr<Layer> parent);
    virtual ~RareBiomeSpotLayer();

    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);
};

#endif
