#ifndef GAME_CORE_RINGSHAPE_HPP
#define GAME_CORE_RINGSHAPE_HPP
#include "game/world/generation/LayerShape.hpp"

class Random;
class Shape;

class RingShape : public LayerShape
{
public:
    float scale;
    float radius;
    LayerShape* layer;

    RingShape(LayerShape* layer, float scale, float radius);

    virtual void GenerateShape(Random& random, Shape& shape) const;
};

#endif
