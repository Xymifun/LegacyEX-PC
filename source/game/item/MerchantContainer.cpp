#include "game/item/MerchantContainer.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantRecipe.hpp"
#include "game/item/MerchantRecipeList.hpp"
#include "game/core/Merchant.hpp"

MerchantContainer::MerchantContainer(boost::shared_ptr<Player> player, boost::shared_ptr<Merchant> merchant)
{
    mTradingPlayer = player;
    mMerchant = merchant;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(3, ItemInstance::EMPTY);
    mActiveRecipe = 0;
    mSelectionHint = 0;
}

MerchantContainer::~MerchantContainer()
{
    delete[] mItems.items.data;
}

std::wstring MerchantContainer::getName()
{
    return mMerchant->getDisplayName();
}

std::wstring MerchantContainer::getCustomName()
{
    return std::wstring(L"");
}

bool MerchantContainer::hasCustomName()
{
    return false;
}

std::wstring MerchantContainer::getDisplayName()
{
    if (hasCustomName())
        return getCustomName();
    return getName();
}

void MerchantContainer::updateSellItem()
{
    mActiveRecipe = 0;
    not_null_ptr<ItemInstance> first = mItems[0];
    not_null_ptr<ItemInstance> second = mItems[1];
    if (first->isEmpty())
    {
        first = second;
        second = ItemInstance::EMPTY;
    }

    if (first->isEmpty())
    {
        setItem(2, ItemInstance::EMPTY);
    }
    else
    {
        MerchantRecipeList* offers = mMerchant->getOffers(mTradingPlayer);
        if (offers != 0)
        {
            MerchantRecipe* recipe = offers->getRecipeFor(first, second, mSelectionHint);
            if (recipe != 0 && !recipe->isDeprecated())
            {
                mActiveRecipe = recipe;
                setItem(2, recipe->getSellItem()->copy());
            }
            else if (!second->isEmpty())
            {
                recipe = offers->getRecipeFor(second, first, mSelectionHint);
                if (recipe != 0 && !recipe->isDeprecated())
                {
                    mActiveRecipe = recipe;
                    setItem(2, recipe->getSellItem()->copy());
                }
                else
                {
                    setItem(2, ItemInstance::EMPTY);
                }
            }
            else
            {
                setItem(2, ItemInstance::EMPTY);
            }
        }
        mMerchant->notifyTradeUpdated(getItem(2));
    }
}

void MerchantContainer::setSelectionHint(int hint)
{
    mSelectionHint = hint;
    updateSellItem();
}

unsigned int MerchantContainer::getContainerSize()
{
    return mItems.size();
}

bool MerchantContainer::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mItems[i];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> MerchantContainer::getItem(unsigned int slot)
{
    return mItems[slot];
}

bool MerchantContainer::isPaymentSlot(int slot)
{
    return slot == 0 || slot == 1;
}

not_null_ptr<ItemInstance> MerchantContainer::removeItem(unsigned int slot, int count)
{
    not_null_ptr<ItemInstance> item = mItems[slot];
    if (slot == 2 && !item->isEmpty())
    {
        return ContainerHelper::removeItem(mItems, slot, (unsigned int)item->getCount());
    }

    not_null_ptr<ItemInstance> removed = ContainerHelper::removeItem(mItems, slot, (unsigned int)count);
    if (!removed->isEmpty() && isPaymentSlot((int)slot))
    {
        updateSellItem();
    }
    return removed;
}

not_null_ptr<ItemInstance> MerchantContainer::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, (unsigned int)slot);
}

void MerchantContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize())
    {
        item->setCount(getMaxStackSize());
    }
    if (isPaymentSlot((int)slot))
    {
        updateSellItem();
    }
}

int MerchantContainer::getMaxStackSize()
{
    return 64;
}

bool MerchantContainer::stillValid(boost::shared_ptr<Player> player)
{
    return mMerchant->getTradingPlayer() == player;
}

void MerchantContainer::startOpen(boost::shared_ptr<Player>)
{
}

void MerchantContainer::stopOpen(boost::shared_ptr<Player>)
{
}

bool MerchantContainer::canPlaceItem(int, not_null_ptr<ItemInstance>)
{
    return true;
}

void MerchantContainer::setChanged()
{
    updateSellItem();
}

MerchantRecipe* MerchantContainer::getActiveRecipe()
{
    return mActiveRecipe;
}

int MerchantContainer::getContainerData(int)
{
    return 0;
}

void MerchantContainer::setContainerData(int, int)
{
}

int MerchantContainer::getContainerDataCount()
{
    return 0;
}

void MerchantContainer::clearContent()
{
    mItems.clear();
}
