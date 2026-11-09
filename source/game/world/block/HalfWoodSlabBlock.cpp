#include "game/world/block/HalfWoodSlabBlock.hpp"

HalfWoodSlabBlock::HalfWoodSlabBlock() : WoodSlabBlock()
{
    WoodSlabBlock::DerivedInit();
}

bool HalfWoodSlabBlock::isFullSize() const
{
    return false;
}
