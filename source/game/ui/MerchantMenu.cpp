#include "game/ui/MerchantMenu.hpp"

#include "game/core/Merchant.hpp"
#include "game/core/MerchantResultSlot.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MerchantContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

MerchantMenu::MerchantMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Merchant> merchant, Level* level)
    : AbstractContainerMenu()
{
    mMerchant = merchant;
    mLevel = level;
    mTradeContainer = boost::shared_ptr<MerchantContainer>(
        new MerchantContainer(boost::dynamic_pointer_cast<Player>(inventory->m_player->shared_from_this()), merchant));
    addSlot(new Slot(mTradeContainer, 0, 36, 53));
    addSlot(new Slot(mTradeContainer, 1, 62, 53));
    addSlot(new MerchantResultSlot(inventory->m_player, merchant, mTradeContainer, 2, 120, 53));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }
}

boost::shared_ptr<MerchantContainer> MerchantMenu::getTradeContainer()
{
    return mTradeContainer;
}

void MerchantMenu::slotsChanged()
{
    mTradeContainer->updateSellItem();
    AbstractContainerMenu::slotsChanged();
}

void MerchantMenu::setSelectionHint(int hint)
{
    mTradeContainer->setSelectionHint(hint);
}

bool MerchantMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mMerchant->getTradingPlayer() == player;
}

not_null_ptr<ItemInstance> MerchantMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    if (static_cast<unsigned int>(index) < m_slots.size())
    {
        Slot* slot = m_slots.at(index);
        if (slot != NULL && slot->hasItem())
        {
            not_null_ptr<ItemInstance> item = slot->getItem();
            result = item->copy();
            if (index == 2)
            {
                if (!moveItemStackTo(item, 3, 39, true, true, true))
                {
                    return ItemInstance::EMPTY;
                }
                slot->onQuickCraft(item, result);
            }
            else if (index == 0 || index == 1)
            {
                if (!moveItemStackTo(item, 3, 39, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (index >= 3 && index < 30)
            {
                if (!moveItemStackTo(item, 30, 39, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (index >= 30 && index < 39)
            {
                if (!moveItemStackTo(item, 3, 30, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            if (item->isEmpty())
            {
                slot->set(ItemInstance::EMPTY);
            }
            else
            {
                slot->setChanged();
            }
            if (item->getCount() == result->getCount())
            {
                return ItemInstance::EMPTY;
            }
            slot->onTake(player, item);
        }
    }
    return result;
}

void MerchantMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    mMerchant->setTradingPlayer(boost::shared_ptr<Player>());
    AbstractContainerMenu::removed(player);
    if (!mLevel->mIsClientSide)
    {
        not_null_ptr<ItemInstance> item = mTradeContainer->removeItemNoUpdate(0);
        if (!item->isEmpty())
        {
            player->drop(item, false);
        }
        item = mTradeContainer->removeItemNoUpdate(1);
        if (!item->isEmpty())
        {
            player->drop(item, false);
        }
    }
}

boost::shared_ptr<Merchant> MerchantMenu::getMerchant()
{
    return mMerchant;
}
