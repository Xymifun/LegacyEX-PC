#ifndef GAME_CORE_STARSHAPE_HPP
#define GAME_CORE_STARSHAPE_HPP
#include "game/world/generation/LayerShape.hpp"

class Random;
class Shape;

class StarShape : public LayerShape
{
public:
    unsigned int points;

    StarShape(unsigned int points);

    virtual void GenerateShape(Random& random, Shape& shape) const;
};

#endif
