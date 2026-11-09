#include "game/core/CircleShape.hpp"

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"

void CircleShape::GenerateShape(Random& random, Shape& shape) const
{
    Vec3i centre(shape.width / 2, 0, shape.depth / 2);
    Vec3i edge(shape.width / 2, 0, 0);
    double radius = centre.dist(edge) - 1.0;

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            shape.at(x, z) = (float)(centre.dist(Vec3i((int)x, 0, (int)z)) / radius);
        }
    }
}
