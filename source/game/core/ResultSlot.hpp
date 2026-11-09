#ifndef GAME_CORE_RESULTSLOT_HPP
#define GAME_CORE_RESULTSLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Slot.hpp"

class CraftingContainer;
class Player;

class ResultSlot : public Slot
{
public:
    ResultSlot(Player* player, boost::shared_ptr<CraftingContainer> craftSlots,
               boost::shared_ptr<Container> resultContainer, int slot, int x, int y);

    virtual void onQuickCraft(not_null_ptr<ItemInstance> item, int count);
    virtual void onSwapCraft(int count);
    virtual void checkTakeAchievements(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> remove(int count);
    virtual bool mayCombine(not_null_ptr<ItemInstance> item);

private:
    boost::shared_ptr<CraftingContainer> mCraftSlots;
    Player* mPlayer;
    int mRemoveCount;
};

#endif
