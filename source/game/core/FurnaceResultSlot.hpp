#ifndef GAME_CORE_FURNACE_RESULT_SLOT_HPP
#define GAME_CORE_FURNACE_RESULT_SLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slot.hpp"

class ItemInstance;
class Player;

class FurnaceResultSlot : public Slot
{
public:
    FurnaceResultSlot(boost::shared_ptr<Player> player, boost::shared_ptr<Container> container, int index, int x,
                      int y);

    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> remove(int count);
    virtual not_null_ptr<ItemInstance> onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
    virtual bool mayCombine(not_null_ptr<ItemInstance> item);
    virtual void onQuickCraft(not_null_ptr<ItemInstance> item, int count);
    virtual void checkTakeAchievements(not_null_ptr<ItemInstance> item);

private:
    boost::shared_ptr<Player> mPlayer;
    int mRemoveCount;
};

#endif
