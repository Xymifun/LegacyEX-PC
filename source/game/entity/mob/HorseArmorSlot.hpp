#ifndef GAME_ENTITY_MOB_HORSEARMORSLOT_HPP
#define GAME_ENTITY_MOB_HORSEARMORSLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slot.hpp"
#include "game/item/not_null_ptr.hpp"

class HorseInventoryMenu;

class HorseArmorSlot : public Slot
{
public:
    HorseArmorSlot(HorseInventoryMenu* menu, boost::shared_ptr<Container> container, int index, int x, int y)
        : Slot(container, index, x, y),
          menu(menu)
    {
    }

    virtual bool mayPlace(not_null_ptr<ItemInstance> itemInstance);
    virtual int getMaxStackSize();
    virtual bool isActive();

private:
    HorseInventoryMenu* menu;
};

#endif
