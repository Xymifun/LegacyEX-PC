#ifndef GAME_WORLD_BLOCK_YELLOWFLOWERBLOCK_HPP
#define GAME_WORLD_BLOCK_YELLOWFLOWERBLOCK_HPP

#include "game/world/block/FlowerBlock.hpp"

class YellowFlowerBlock : public FlowerBlock
{
public:
    YellowFlowerBlock();

    virtual FlowerBlock::FlowerCategory* getFlowerCategory();
};

#endif
