#include "game/core/Shape.hpp"

Shape::Shape(unsigned int width, unsigned int depth)
{
    unsigned int count = width * depth;

    values = new float[count];
    this->width = width;
    this->depth = depth;

    for (unsigned int i = 0; i < count; i++)
    {
        values[i] = -1.0f;
    }
}

Shape::~Shape()
{
    delete[] values;
}

float& Shape::at(unsigned int x, unsigned int z)
{
    return values[width * x + z];
}

const float& Shape::at(unsigned int x, unsigned int z) const
{
    return values[width * x + z];
}
