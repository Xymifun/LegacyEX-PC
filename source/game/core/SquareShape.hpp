#ifndef GAME_CORE_SQUARESHAPE_HPP
#define GAME_CORE_SQUARESHAPE_HPP
#include "game/world/generation/LayerShape.hpp"

class Random;
class Shape;

class SquareShape : public LayerShape
{
public:
    virtual void GenerateShape(Random& random, Shape& shape) const;
};

#endif
