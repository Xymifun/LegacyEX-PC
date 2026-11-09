#ifndef GAME_WORLD_BLOCK_REDFLOWERBLOCK_HPP
#define GAME_WORLD_BLOCK_REDFLOWERBLOCK_HPP

#include "game/world/block/FlowerBlock.hpp"

class RedFlowerBlock : public FlowerBlock
{
public:
    RedFlowerBlock();

    virtual FlowerBlock::FlowerCategory* getFlowerCategory();
};

#endif
