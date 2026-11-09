#ifndef GAME_CORE_MERCHANTRESULTSLOT_HPP
#define GAME_CORE_MERCHANTRESULTSLOT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Merchant.hpp"
#include "game/core/Slot.hpp"
#include "game/item/MerchantContainer.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class MerchantRecipe;
class Player;

class MerchantResultSlot : public Slot
{
public:
    MerchantResultSlot(Player* player, boost::shared_ptr<Merchant> merchant,
                       boost::shared_ptr<MerchantContainer> merchantContainer, int slot, int x, int y);

    virtual bool mayPlace(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> remove(int count);
    virtual void onQuickCraft(not_null_ptr<ItemInstance> item, int count);
    virtual void checkTakeAchievements(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item);
    virtual bool mayCombine(not_null_ptr<ItemInstance> item);

private:
    boost::shared_ptr<MerchantContainer> mMerchantContainer;
    Player* mPlayer;
    int mRemoveCount;
    boost::shared_ptr<Merchant> mMerchant;

    bool removePaymentItemsIfMatching(MerchantRecipe* recipe, not_null_ptr<ItemInstance> first,
                                      not_null_ptr<ItemInstance> second);
};

#endif
