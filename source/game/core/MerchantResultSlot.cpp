#include "game/core/MerchantResultSlot.hpp"

#include <algorithm>

#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/world/generation/Player.hpp"

MerchantResultSlot::MerchantResultSlot(Player* player, boost::shared_ptr<Merchant> merchant,
                                       boost::shared_ptr<MerchantContainer> merchantContainer, int slot, int x, int y)
    : Slot(merchantContainer, slot, x, y)
{
    mPlayer = player;
    mMerchant = merchant;
    mMerchantContainer = merchantContainer;
    mRemoveCount = 0;
}

bool MerchantResultSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return false;
}

not_null_ptr<ItemInstance> MerchantResultSlot::remove(int count)
{
    if (hasItem())
    {
        mRemoveCount += std::min(count, getItem()->getCount());
    }
    return Slot::remove(count);
}

void MerchantResultSlot::onQuickCraft(not_null_ptr<ItemInstance> item, int count)
{
    mRemoveCount += count;
    checkTakeAchievements(item);
}

void MerchantResultSlot::checkTakeAchievements(not_null_ptr<ItemInstance> item)
{
    item->onCraftedBy(mPlayer->mLevel, boost::static_pointer_cast<Player>(mPlayer->shared_from_this()), mRemoveCount);
    mRemoveCount = 0;
}

bool MerchantResultSlot::removePaymentItemsIfMatching(MerchantRecipe* recipe, not_null_ptr<ItemInstance> first,
                                                      not_null_ptr<ItemInstance> second)
{
    not_null_ptr<ItemInstance> buyA = recipe->getBuyAItem();
    not_null_ptr<ItemInstance> buyB = recipe->getBuyBItem();

    if (first->getItem() == buyA->getItem() && first->getCount() >= buyA->getCount())
    {
        if (!buyB->isEmpty() && !second->isEmpty() && buyB->getItem() == second->getItem()
            && second->getCount() >= buyB->getCount())
        {
            first->shrink(buyA->getCount());
            second->shrink(buyB->getCount());
            return true;
        }
        else if (buyB->isEmpty() && second->isEmpty())
        {
            first->shrink(buyA->getCount());
            return true;
        }
    }

    return false;
}

not_null_ptr<ItemInstance> MerchantResultSlot::onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item)
{
    checkTakeAchievements(item);

    MerchantRecipe* recipe = mMerchantContainer->getActiveRecipe();
    if (recipe != NULL)
    {
        not_null_ptr<ItemInstance> first = mMerchantContainer->getItem(0);
        not_null_ptr<ItemInstance> second = mMerchantContainer->getItem(1);
        if (removePaymentItemsIfMatching(recipe, first, second) || removePaymentItemsIfMatching(recipe, second, first))
        {
            mMerchant->notifyTrade(recipe);
            mMerchantContainer->setItem(0, first);
            mMerchantContainer->setItem(1, second);
        }
    }

    return item;
}

bool MerchantResultSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}
