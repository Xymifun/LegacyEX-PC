#ifndef GAME_ITEM_BOOKITEM_HPP
#define GAME_ITEM_BOOKITEM_HPP

#include "game/item/Item.hpp"

class BookItem : public Item
{
public:
    BookItem();
    virtual bool isEnchantable(not_null_ptr<ItemInstance> itemInstance);
    virtual int getEnchantmentValue();
};

#endif
