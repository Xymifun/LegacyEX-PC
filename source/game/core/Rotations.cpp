#include "game/core/Rotations.hpp"

#include "game/core/Mth.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/ListTag.hpp"

Rotations::Rotations(float x, float y, float z) : x(x), y(y), z(z)
{
}

Rotations::Rotations(const ListTag* tag) : x(tag->getFloat(0)), y(tag->getFloat(1)), z(tag->getFloat(2))
{
}

ListTag* Rotations::save() const
{
    ListTag* tag = new ListTag();
    tag->add(new FloatTag(x));
    tag->add(new FloatTag(y));
    tag->add(new FloatTag(z));
    return tag;
}

bool Rotations::equals(const Rotations* other) const
{
    return x == other->x && y == other->y && z == other->z;
}

float Rotations::getX() const
{
    return x;
}

float Rotations::getY() const
{
    return y;
}

float Rotations::getZ() const
{
    return z;
}

float Rotations::getWrappedX() const
{
    return Mth::wrapDegrees(x);
}

float Rotations::getWrappedY() const
{
    return Mth::wrapDegrees(y);
}

float Rotations::getWrappedZ() const
{
    return Mth::wrapDegrees(z);
}
