#ifndef GAME_CORE_LAYERSIZE_HPP
#define GAME_CORE_LAYERSIZE_HPP

#include "game/util/Random.hpp"

class LayerSize
{
public:
    virtual unsigned int GetLayerSize(Random& random, double blend) const = 0;
};

#endif
