#ifndef GAME_WORLD_BLOCK_ANVILBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_ANVILBLOCKITEM_HPP

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/MultiBlockItem.hpp"

class Block;
class ItemInstance;

class AnvilBlockItem : public MultiBlockItem
{
public:
    explicit AnvilBlockItem(Block* block);

    static int AnvilBlockNameResolver(int damage);
    static int AnvilBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);

    virtual int getLevelDataForAuxValue(int auxValue);
    virtual int getDescriptionId(int auxValue);
};

#endif
