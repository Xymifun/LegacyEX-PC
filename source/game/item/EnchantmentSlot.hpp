#ifndef GAME_ITEM_ENCHANTMENTSLOT_HPP
#define GAME_ITEM_ENCHANTMENTSLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slot.hpp"
#include "game/item/Container.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;

class EnchantmentSlot : public Slot
{
public:
    EnchantmentSlot(boost::shared_ptr<Container> container, int index, int x, int y) : Slot(container, index, x, y) {}

    virtual bool mayPlace(not_null_ptr<ItemInstance> itemInstance);
    virtual int getMaxStackSize();
    virtual bool mayCombine(not_null_ptr<ItemInstance> itemInstance);
};

#endif
