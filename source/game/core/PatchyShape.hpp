#ifndef GAME_CORE_PATCHYSHAPE_HPP
#define GAME_CORE_PATCHYSHAPE_HPP
#include "game/world/generation/LayerShape.hpp"

class Random;
class Shape;

class PatchyShape : public LayerShape
{
public:
    PatchyShape(double scale);

    virtual void GenerateShape(Random& random, Shape& shape) const;

    double scale;
};

#endif
