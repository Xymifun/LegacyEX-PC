#ifndef GAME_WORLD_BLOCK_FULLNEWSTONESLABBLOCK_HPP
#define GAME_WORLD_BLOCK_FULLNEWSTONESLABBLOCK_HPP

#include "game/world/block/NewStoneSlabBlock.hpp"

class FullNewStoneSlabBlock : public NewStoneSlabBlock
{
public:
    FullNewStoneSlabBlock();
    virtual bool isFullSize() const;
};

#endif
