#ifndef GAME_WORLD_GENERATION_FUZZYZOOMLAYER_HPP
#define GAME_WORLD_GENERATION_FUZZYZOOMLAYER_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/generation/ZoomLayer.hpp"

class FuzzyZoomLayer : public ZoomLayer
{
public:
    static boost::shared_ptr<Layer> zoom(long long seed, boost::shared_ptr<Layer> parent, int count);

    FuzzyZoomLayer(long long seed, boost::shared_ptr<Layer> parent);

    virtual int modeOrRandom(int upperLeft, int upperRight, int lowerLeft, int lowerRight);
};

#endif
