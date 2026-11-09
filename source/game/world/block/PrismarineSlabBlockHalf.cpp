#include "game/world/block/PrismarineSlabBlockHalf.hpp"

PrismarineSlabBlockHalf::PrismarineSlabBlockHalf() : PrismarineSlabBlock()
{
    PrismarineSlabBlock::DerivedInit();
}

bool PrismarineSlabBlockHalf::isFullSize() const
{
    return false;
}
