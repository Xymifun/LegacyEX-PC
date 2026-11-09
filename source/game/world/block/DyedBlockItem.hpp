#ifndef GAME_WORLD_BLOCK_DYEDBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_DYEDBLOCKITEM_HPP

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockItem.hpp"

class Block;
class ItemInstance;

class DyedBlockItem : public BlockItem
{
public:
    DyedBlockItem(Block* block);

    virtual ~DyedBlockItem();

    virtual int getLevelDataForAuxValue(int auxValue);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
};

#endif
