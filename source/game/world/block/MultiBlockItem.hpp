#ifndef GAME_WORLD_BLOCK_MULTIBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_MULTIBLOCKITEM_HPP

#include "game/world/block/BlockItem.hpp"

class Block;
class ItemInstance;

class MultiBlockItem : public BlockItem
{
public:
    typedef int (*ItemInstanceResolver)(not_null_ptr<ItemInstance> itemInstance);

    MultiBlockItem(Block* block, Block* blockForAuxValue, ItemInstanceResolver nameFunction,
                   ItemInstanceResolver descriptionFunction);
    virtual ~MultiBlockItem() {}

    virtual int getLevelDataForAuxValue(int auxValue);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);

private:
    Block* mBlockForAuxValue;
    ItemInstanceResolver mNameFunction;
    ItemInstanceResolver mDescriptionFunction;
};

#endif
