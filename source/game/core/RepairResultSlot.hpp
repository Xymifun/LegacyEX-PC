#ifndef GAME_CORE_REPAIRRESULTSLOT_HPP
#define GAME_CORE_REPAIRRESULTSLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slot.hpp"
#include "game/world/block/BlockPos.hpp"

class AnvilMenu;
class Container;
class ItemInstance;
class Player;

class RepairResultSlot : public Slot
{
public:
    RepairResultSlot(AnvilMenu* menu, const BlockPos& position, boost::shared_ptr<Container> container, int index,
                     int x, int y);

    virtual not_null_ptr<ItemInstance> onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual bool mayPickup(boost::shared_ptr<Player> player);
    virtual bool mayCombine(not_null_ptr<ItemInstance> item);

private:
    AnvilMenu* mMenu;
    BlockPos mPosition;
};

#endif
