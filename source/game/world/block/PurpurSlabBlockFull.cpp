#include "game/world/block/PurpurSlabBlockFull.hpp"

PurpurSlabBlockFull::PurpurSlabBlockFull() : PurpurSlabBlock()
{
    PurpurSlabBlock::DerivedInit();
}

bool PurpurSlabBlockFull::isFullSize() const
{
    return true;
}
