#ifndef GAME_WORLD_GENERATION_SHORELAYER_HPP
#define GAME_WORLD_GENERATION_SHORELAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/Layer.hpp"

class ShoreLayer : public Layer
{
public:
    ShoreLayer(long long seed, boost::shared_ptr<Layer> parent);
    virtual arrayWithLength<int> getArea(int x, int z, int width, int height);

private:
    bool isJungleCompatible(int biomeId);
    void replaceIfNeighborOcean(arrayWithLength<int> parentArea, arrayWithLength<int> area, int x, int z, int width,
                                int centre, int replacement);
    bool isMesa(int biomeId);
};

#endif
