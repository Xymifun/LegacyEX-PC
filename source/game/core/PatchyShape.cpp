#include "game/core/PatchyShape.hpp"

#include "game/core/Shape.hpp"
#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"

PatchyShape::PatchyShape(double scale) : scale(scale)
{
}

void PatchyShape::GenerateShape(Random& random, Shape& shape) const
{
    SimplexNoise noise(&random);

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            double xd = (double)x / this->scale;
            float value = (float)(noise.getValue(xd, (double)z / this->scale) + 1.0);
            shape.at(x, z) = value;
        }
    }
}
