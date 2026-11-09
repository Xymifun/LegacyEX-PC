#ifndef GAME_ITEM_SIMPLEFOILEDITEM_HPP
#define GAME_ITEM_SIMPLEFOILEDITEM_HPP

#include "game/item/Item.hpp"

class SimpleFoiledItem : public Item
{
public:
    SimpleFoiledItem();
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
};

#endif
