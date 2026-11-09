#ifndef GAME_WORLD_BLOCK_HALFNEWSTONESLABBLOCK_HPP
#define GAME_WORLD_BLOCK_HALFNEWSTONESLABBLOCK_HPP

#include "game/world/block/NewStoneSlabBlock.hpp"

class HalfNewStoneSlabBlock : public NewStoneSlabBlock
{
public:
    HalfNewStoneSlabBlock();
    virtual bool isFullSize() const;
};

#endif
