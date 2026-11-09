#include "game/world/block/FullNewStoneSlabBlock.hpp"

FullNewStoneSlabBlock::FullNewStoneSlabBlock() : NewStoneSlabBlock()
{
    NewStoneSlabBlock::DerivedInit();
}

bool FullNewStoneSlabBlock::isFullSize() const
{
    return true;
}
