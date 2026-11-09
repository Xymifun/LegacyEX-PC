#ifndef GAME_ITEM_ITEMCOLORS_HPP
#define GAME_ITEM_ITEMCOLORS_HPP
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/IdMapper.hpp"
class Block;
class BlockColors;
class Item;
class ItemInstance;
class ItemColor
{
public:
    virtual int getColor(const not_null_ptr<ItemInstance>& item, int layer) = 0;
};

class ItemColors
{
public:
    ItemColors();
    void registerColor(ItemColor* color, arrayWithLength<Item*> items);
    void registerColor(ItemColor* color, arrayWithLength<Block*> blocks);
    static ItemColors* createDefault(BlockColors* blockColors);
    int getColor(not_null_ptr<ItemInstance> item, int layer);

private:
    IdMapper<ItemColor*>* mItemColorIds;
};
#endif
