#include "game/core/CustomShape.hpp"

#include <math.h>

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"

CustomShape::CustomShape(unsigned short size, arrayWithLength<float> values) : size(size), values(values)
{
}

void CustomShape::GenerateShape(Random& random, Shape& shape) const
{
    float xScale = (float)size / (float)shape.width;
    float zScale = (float)size / (float)shape.depth;

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            float scaledX = (float)x * xScale;
            float scaledZ = (float)z * zScale;
            int sourceX = (int)floorf(scaledX);
            int sourceZ = (int)floorf(scaledZ);

            shape.at(x, z) = values.data[size * sourceX + sourceZ];
        }
    }
}
