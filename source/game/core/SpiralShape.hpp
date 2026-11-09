#ifndef GAME_CORE_SPIRALSHAPE_HPP
#define GAME_CORE_SPIRALSHAPE_HPP
#include "game/world/generation/LayerShape.hpp"

class Random;
class Shape;

class SpiralShape : public LayerShape
{
public:
    int minRadius;
    int maxRadius;
    double angle;

    SpiralShape(int minRadius, int maxRadius, double angle);

    virtual void GenerateShape(Random& random, Shape& shape) const;
};

#endif
