#include "game/world/block/PurpurSlabBlockHalf.hpp"

PurpurSlabBlockHalf::PurpurSlabBlockHalf() : PurpurSlabBlock()
{
    PurpurSlabBlock::DerivedInit();
}

bool PurpurSlabBlockHalf::isFullSize() const
{
    return false;
}
