#include "game/core/TemplateShape.hpp"

void TemplateShape::GenerateShape(Random& random, Shape& shape) const
{
    unsigned int x;
    unsigned int z;
    for (x = 0; x < shape.width; ++x)
    {
        for (z = 0; z < shape.depth; ++z)
        {
            shape.at(x, z) = 0.5f;
        }
    }
}
