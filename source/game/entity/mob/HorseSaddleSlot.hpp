#ifndef GAME_ENTITY_MOB_HORSESADDLESLOT_HPP
#define GAME_ENTITY_MOB_HORSESADDLESLOT_HPP

#include "game/core/Slot.hpp"

class HorseInventoryMenu;

class HorseSaddleSlot : public Slot
{
public:
    HorseSaddleSlot(HorseInventoryMenu* menu, boost::shared_ptr<Container> container, int index, int x, int y)
        : Slot(container, index, x, y),
          mMenu(menu)
    {
    }

    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual bool isActive();

private:
    HorseInventoryMenu* mMenu;
};

#endif
