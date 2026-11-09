#ifndef GAME_CORE_TEMPLATESHAPE_HPP
#define GAME_CORE_TEMPLATESHAPE_HPP

#include "game/world/generation/LayerShape.hpp"

class TemplateShape : public LayerShape
{
public:
    virtual void GenerateShape(Random& random, Shape& shape) const;
};

#endif
