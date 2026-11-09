#include "game/world/block/HalfNewStoneSlabBlock.hpp"

HalfNewStoneSlabBlock::HalfNewStoneSlabBlock() : NewStoneSlabBlock()
{
    NewStoneSlabBlock::DerivedInit();
}

bool HalfNewStoneSlabBlock::isFullSize() const
{
    return false;
}
