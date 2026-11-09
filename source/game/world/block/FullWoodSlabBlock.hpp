#ifndef GAME_WORLD_BLOCK_FULLWOODSLABBLOCK_HPP
#define GAME_WORLD_BLOCK_FULLWOODSLABBLOCK_HPP

#include "game/world/block/WoodSlabBlock.hpp"

class FullWoodSlabBlock : public WoodSlabBlock
{
public:
    FullWoodSlabBlock();
    virtual bool isFullSize() const;
};

#endif
