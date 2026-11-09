#ifndef GAME_CORE_CIRCLESHAPE_HPP
#define GAME_CORE_CIRCLESHAPE_HPP
#include "game/world/generation/LayerShape.hpp"

class Random;
class Shape;

class CircleShape : public LayerShape
{
public:
    virtual void GenerateShape(Random& random, Shape& shape) const;
};

#endif
