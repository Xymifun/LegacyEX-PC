#include "game/core/DoorInfo.hpp"

DoorInfo::DoorInfo(const BlockPos& pos, int dx, int dz, int timeStamp) : BlockPos(), indoorPos()
{
    _init(pos, facingFromDelta(dx, dz), timeStamp);
}

DoorInfo::DoorInfo(const BlockPos& pos, const Direction* direction, int timeStamp) : BlockPos(), indoorPos()
{
    _init(pos, direction, timeStamp);
}

void DoorInfo::_init(const BlockPos& pos, const Direction* direction, int timeStamp)
{
    BlockPos::operator=(pos);
    this->direction = direction;
    indoorPos = pos.relative(direction, 2);
    this->timeStamp = timeStamp;
}

const Direction* DoorInfo::facingFromDelta(int dx, int dz)
{
    if (dx < 0)
        return Direction::WEST;
    if (dx > 0)
        return Direction::EAST;
    if (dz < 0)
        return Direction::NORTH;
    return Direction::SOUTH;
}

int DoorInfo::distanceToSqr(int x, int y, int z)
{
    return static_cast<int>(Vec3i::distSqr(static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)));
}

int DoorInfo::distanceToSqr(const BlockPos& pos)
{
    return static_cast<int>(indoorPos.distSqr(pos));
}

int DoorInfo::distanceToInsideSqr(const BlockPos& pos)
{
    return static_cast<int>(indoorPos.distSqr(pos));
}

bool DoorInfo::isInsideSide(const BlockPos& pos)
{
    int dx = pos.x - x;
    int dz = pos.z - z;
    return dx * direction->getStepX() + dz * direction->getStepZ() >= 0;
}

void DoorInfo::resetBookingCount()
{
    bookingsCount = 0;
}

void DoorInfo::incBookingCount()
{
    ++bookingsCount;
}

int DoorInfo::getBookingsCount()
{
    return bookingsCount;
}

BlockPos DoorInfo::getPos()
{
    return BlockPos(*this);
}

BlockPos DoorInfo::getIndoorPos()
{
    return indoorPos;
}

int DoorInfo::getInsideDx()
{
    return direction->getStepX() * 2;
}

int DoorInfo::getInsideDz()
{
    return direction->getStepZ() * 2;
}

int DoorInfo::getTimeStamp()
{
    return timeStamp;
}

void DoorInfo::setTimeStamp(int timeStamp)
{
    this->timeStamp = timeStamp;
}

bool DoorInfo::isRemoved()
{
    return removed;
}

void DoorInfo::setRemoved(bool removed)
{
    this->removed = removed;
}

const Direction* DoorInfo::getDirection()
{
    return direction;
}

double DoorInfo::distanceTo(int x, int y, int z)
{
    return Vec3i::dist(x, y, z);
}
