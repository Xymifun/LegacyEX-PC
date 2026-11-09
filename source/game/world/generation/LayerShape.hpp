#ifndef GAME_WORLD_GENERATION_LAYERSHAPE_HPP
#define GAME_WORLD_GENERATION_LAYERSHAPE_HPP

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"

class LayerShape
{
public:
    virtual void GenerateShape(Random& random, Shape& shape) const = 0;
};

#endif
