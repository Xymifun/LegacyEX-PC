#ifndef GAME_CORE_DOORINFO_HPP
#define GAME_CORE_DOORINFO_HPP

#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"

class DoorInfo : public BlockPos
{
public:
    DoorInfo(const BlockPos& pos, int dx, int dz, int timeStamp);
    DoorInfo(const BlockPos& pos, const Direction* direction, int timeStamp);
    void _init(const BlockPos& pos, const Direction* direction, int timeStamp);
    static const Direction* facingFromDelta(int dx, int dz);
    int distanceToSqr(int x, int y, int z);
    int distanceToSqr(const BlockPos& pos);
    int distanceToInsideSqr(const BlockPos& pos);
    bool isInsideSide(const BlockPos& pos);
    void resetBookingCount();
    void incBookingCount();
    int getBookingsCount();
    BlockPos getPos();
    BlockPos getIndoorPos();
    int getInsideDx();
    int getInsideDz();
    int getTimeStamp();
    void setTimeStamp(int timeStamp);
    bool isRemoved();
    void setRemoved(bool removed);
    const Direction* getDirection();
    double distanceTo(int x, int y, int z);

private:
    BlockPos indoorPos;
    const Direction* direction;
    int timeStamp;
    bool removed;
    int bookingsCount;
};

#endif
