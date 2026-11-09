#include "game/world/block/FullWoodSlabBlock.hpp"

FullWoodSlabBlock::FullWoodSlabBlock() : WoodSlabBlock()
{
    WoodSlabBlock::DerivedInit();
}

bool FullWoodSlabBlock::isFullSize() const
{
    return true;
}
