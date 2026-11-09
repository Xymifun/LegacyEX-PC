#ifndef GAME_WORLD_BLOCK_LEAFBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_LEAFBLOCKITEM_HPP

#include "game/world/block/BlockItem.hpp"

class ItemInstance;
class LeafBlock;

class LeafBlockItem : public BlockItem
{
public:
    explicit LeafBlockItem(LeafBlock* block);

    virtual int getLevelDataForAuxValue(int auxValue);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);

private:
    LeafBlock* mLeafBlock;
};

#endif
