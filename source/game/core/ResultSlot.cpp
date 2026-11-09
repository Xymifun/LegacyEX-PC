#include "game/core/ResultSlot.hpp"

#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/generation/Player.hpp"

ResultSlot::ResultSlot(Player* player, boost::shared_ptr<CraftingContainer> craftSlots,
                       boost::shared_ptr<Container> resultContainer, int slot, int x, int y)
    : Slot(resultContainer, slot, x, y),
      mPlayer(player)
{
    mCraftSlots = craftSlots;
    mRemoveCount = 0;
}

bool ResultSlot::mayPlace(not_null_ptr<ItemInstance>)
{
    return false;
}

not_null_ptr<ItemInstance> ResultSlot::remove(int count)
{
    if (hasItem())
    {
        not_null_ptr<ItemInstance> item = getItem();
        int available = item->getCount();
        mRemoveCount += count < available ? count : available;
    }
    return Slot::remove(count);
}

void ResultSlot::onQuickCraft(not_null_ptr<ItemInstance> item, int count)
{
    mRemoveCount += count;
    checkTakeAchievements(item);
}

void ResultSlot::onSwapCraft(int count)
{
    mRemoveCount += count;
}

void ResultSlot::checkTakeAchievements(not_null_ptr<ItemInstance> item)
{
    if (mRemoveCount > 0)
    {
        item->onCraftedBy(mPlayer->mLevel, boost::static_pointer_cast<Player>(mPlayer->shared_from_this()),
                          mRemoveCount);
    }
    mRemoveCount = 0;
}

not_null_ptr<ItemInstance> ResultSlot::onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> item)
{
    checkTakeAchievements(item);

    NonNullList<not_null_ptr<ItemInstance> > remaining = Recipes::getInstance()->getRemainingItemsFor(
        mCraftSlots, player->mLevel, Recipes::eRecipeCategory_Crafting);
    unsigned int index;
    for (index = 0; index < remaining.size(); ++index)
    {
        not_null_ptr<ItemInstance> current = mCraftSlots->getItem(index);
        not_null_ptr<ItemInstance> remainder = remaining[index];
        if (!current->isEmpty())
        {
            mCraftSlots->removeItem(index, 1);
            current = mCraftSlots->getItem(index);
        }
        if (!remainder->isEmpty())
        {
            if (current->isEmpty())
            {
                mCraftSlots->setItem(index, remainder);
            }
            else if (ItemInstance::isSame(current, remainder) && ItemInstance::tagMatches(current, remainder))
            {
                remainder->grow(current->getCount());
                mCraftSlots->setItem(index, remainder);
            }
            else if (!mPlayer->mInventory->add(remainder))
            {
                mPlayer->drop(remainder, false);
            }
        }
    }

    return item;
}

bool ResultSlot::mayCombine(not_null_ptr<ItemInstance>)
{
    return false;
}
