#include "game/core/SpiralShape.hpp"

SpiralShape::SpiralShape(int minRadius, int maxRadius, double angle)
    : minRadius(minRadius),
      maxRadius(maxRadius),
      angle(angle)
{
}

void SpiralShape::GenerateShape(Random& random, Shape& shape) const
{
    int radius = random.nextInt(this->maxRadius - this->minRadius) + this->minRadius;
}
