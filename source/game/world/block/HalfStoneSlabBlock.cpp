#include "game/world/block/HalfStoneSlabBlock.hpp"

HalfStoneSlabBlock::HalfStoneSlabBlock() : StoneSlabBlock()
{
    StoneSlabBlock::DerivedInit();
}

bool HalfStoneSlabBlock::isFullSize() const
{
    return false;
}
