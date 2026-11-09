#ifndef GAME_WORLD_BLOCK_FULLSTONESLABBLOCK_HPP
#define GAME_WORLD_BLOCK_FULLSTONESLABBLOCK_HPP

#include "game/world/block/StoneSlabBlock.hpp"

class FullStoneSlabBlock : public StoneSlabBlock
{
public:
    FullStoneSlabBlock();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual bool isFullSize() const;
};

#endif
