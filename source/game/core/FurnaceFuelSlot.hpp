#ifndef GAME_CORE_FURNACEFUELSLOT_HPP
#define GAME_CORE_FURNACEFUELSLOT_HPP

#include "game/core/Slot.hpp"

class FurnaceFuelSlot : public Slot
{
public:
    FurnaceFuelSlot(boost::shared_ptr<Container> container, int index, int x, int y);
    virtual ~FurnaceFuelSlot();

    static bool isBucket(not_null_ptr<ItemInstance> item);
    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual int getMaxStackSize(not_null_ptr<ItemInstance> item);
};

#endif
