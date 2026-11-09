#ifndef GAME_WORLD_BLOCK_SHULKERBOXBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_SHULKERBOXBLOCKITEM_HPP

#include "game/world/block/BlockItem.hpp"

class Block;

class ShulkerBoxBlockItem : public BlockItem
{
public:
    explicit ShulkerBoxBlockItem(Block* block);
};

#endif
