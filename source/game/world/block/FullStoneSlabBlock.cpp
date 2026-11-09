#include "game/world/block/FullStoneSlabBlock.hpp"

FullStoneSlabBlock::FullStoneSlabBlock() : StoneSlabBlock()
{
    StoneSlabBlock::DerivedInit();
}

EBlockFaceShape FullStoneSlabBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                      const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_SOLID;
}

bool FullStoneSlabBlock::isFullSize() const
{
    return true;
}
