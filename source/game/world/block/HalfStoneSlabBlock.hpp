#ifndef GAME_WORLD_BLOCK_HALFSTONESLABBLOCK_HPP
#define GAME_WORLD_BLOCK_HALFSTONESLABBLOCK_HPP

#include "game/world/block/StoneSlabBlock.hpp"

class HalfStoneSlabBlock : public StoneSlabBlock
{
public:
    HalfStoneSlabBlock();
    virtual bool isFullSize() const;
};

#endif
