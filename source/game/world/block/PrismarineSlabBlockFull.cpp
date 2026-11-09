#include "game/world/block/PrismarineSlabBlockFull.hpp"

PrismarineSlabBlockFull::PrismarineSlabBlockFull() : PrismarineSlabBlock()
{
    PrismarineSlabBlock::DerivedInit();
}

bool PrismarineSlabBlockFull::isFullSize() const
{
    return true;
}
