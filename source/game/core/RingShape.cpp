#include "game/core/RingShape.hpp"

#include <math.h>

#include "game/core/Shape.hpp"

RingShape::RingShape(LayerShape* layer, float scale, float radius) : scale(scale), radius(radius), layer(layer)
{
}

void RingShape::GenerateShape(Random& random, Shape& shape) const
{
    Shape childShape(shape.width, shape.depth);

    this->layer->GenerateShape(random, childShape);

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            shape.at(x, z) = this->scale * fabsf(childShape.at(x, z) - this->radius);
        }
    }
}
