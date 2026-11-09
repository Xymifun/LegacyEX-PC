#ifndef GAME_UTIL_RANDOMIZESIZE_HPP
#define GAME_UTIL_RANDOMIZESIZE_HPP

#include "game/core/LayerSize.hpp"

class RandomizeSize : public LayerSize
{
public:
    RandomizeSize(LayerSize* layerSize, float amount);

    virtual unsigned int GetLayerSize(Random& random, double blend) const;

private:
    LayerSize* mLayerSize;
    float mAmount;
};

#endif
