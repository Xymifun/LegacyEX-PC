#ifndef GAME_WORLD_BLOCK_PISTONBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_PISTONBLOCKITEM_HPP

#include "game/world/block/BlockItem.hpp"

class Block;

class PistonBlockItem : public BlockItem
{
public:
    explicit PistonBlockItem(Block* block);

    virtual int getLevelDataForAuxValue(int auxValue);
};

#endif
