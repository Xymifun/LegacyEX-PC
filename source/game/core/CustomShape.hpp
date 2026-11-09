#ifndef GAME_CORE_CUSTOMSHAPE_HPP
#define GAME_CORE_CUSTOMSHAPE_HPP

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/LayerShape.hpp"

class Random;
class Shape;

class CustomShape : public LayerShape
{
public:
    unsigned short size;
    arrayWithLength<float> values;

    CustomShape(unsigned short size, arrayWithLength<float> values);

    virtual void GenerateShape(Random& random, Shape& shape) const;
};

#endif
