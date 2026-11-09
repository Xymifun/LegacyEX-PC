#ifndef GAME_ITEM_ENCHANTMENTCOSTSLOT_HPP
#define GAME_ITEM_ENCHANTMENTCOSTSLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slot.hpp"

class ItemInstance;

class EnchantmentCostSlot : public Slot
{
public:
    EnchantmentCostSlot(boost::shared_ptr<Container> container, int index, int x, int y);

    virtual bool mayPlace(not_null_ptr<ItemInstance> itemInstance);
};

#endif
