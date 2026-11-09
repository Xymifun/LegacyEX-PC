#ifndef GAME_CORE_SHULKERBOXSLOT_HPP
#define GAME_CORE_SHULKERBOXSLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slot.hpp"
#include "game/item/not_null_ptr.hpp"

class Container;
class ItemInstance;

class ShulkerBoxSlot : public Slot
{
public:
    ShulkerBoxSlot(boost::shared_ptr<Container> container, int index, int x, int y);

    virtual bool mayPlace(not_null_ptr<ItemInstance> itemInstance);
};

#endif
