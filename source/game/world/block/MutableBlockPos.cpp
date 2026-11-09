#include "game/world/block/MutableBlockPos.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Vec3i.hpp"

MutableBlockPos::MutableBlockPos(int x, int y, int z) : BlockPos(x, y, z)
{
}

MutableBlockPos::MutableBlockPos(const Vec3i& other) : BlockPos(other)
{
}

void MutableBlockPos::setX(int x)
{
    this->x = x;
}

void MutableBlockPos::setY(int y)
{
    this->y = y;
}

void MutableBlockPos::setZ(int z)
{
    this->z = z;
}

MutableBlockPos* MutableBlockPos::set(const BlockPos& pos)
{
    x = pos.x;
    y = pos.y;
    z = pos.z;

    return this;
}

void MutableBlockPos::addX(int dx)
{
    x += dx;
}

void MutableBlockPos::addY(int dy)
{
    y += dy;
}

void MutableBlockPos::addZ(int dz)
{
    z += dz;
}

MutableBlockPos* MutableBlockPos::move(const Direction* direction, int steps)
{
    BlockPos current(*this);
    set(current.offset(direction->getNormal().multiply(steps)));

    return this;
}

MutableBlockPos* MutableBlockPos::move(const Direction* direction)
{
    return move(direction, 1);
}

void MutableBlockPos::add(int dx, int dy, int dz)
{
    x += dx;
    y += dy;
    z += dz;
}
