#ifndef GAME_WORLD_BLOCK_HALFWOODSLABBLOCK_HPP
#define GAME_WORLD_BLOCK_HALFWOODSLABBLOCK_HPP

#include "game/world/block/WoodSlabBlock.hpp"

class HalfWoodSlabBlock : public WoodSlabBlock
{
public:
    HalfWoodSlabBlock();
    virtual bool isFullSize() const;
};

#endif
