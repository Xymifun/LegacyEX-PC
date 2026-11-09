#include "game/ui/FurnaceMenu.hpp"

#include "game/core/FurnaceFuelSlot.hpp"
#include "game/core/FurnaceResultSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/generation/Player.hpp"

FurnaceMenu::FurnaceMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
    : AbstractContainerMenu()
{
    mLitDuration = 0;
    mLitTime = 0;
    mCookingTotalTime = 0;
    mCookingProgress = 0;
    mContainer = container;
    container->startOpen(boost::static_pointer_cast<Player>(inventory->m_player->shared_from_this()));
    addSlot(new Slot(container, 0, 56, 17));
    addSlot(new FurnaceFuelSlot(container, 1, 56, 53));
    addSlot(new FurnaceResultSlot(boost::static_pointer_cast<Player>(inventory->m_player->shared_from_this()),
                                  container, 2, 116, 35));

    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
        }
    }
    for (int column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }
}

void FurnaceMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setAllContainerData(this, mContainer);
}

void FurnaceMenu::broadcastChanges()
{
    AbstractContainerMenu::broadcastChanges();
    std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator end = m_listeners.end();
    std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
    for (; it != end; ++it)
    {
        net_minecraft_world_inventory::ContainerListener* listener = *it;
        if (mCookingProgress != mContainer->getContainerData(2))
        {
            listener->setContainerData(this, 2, mContainer->getContainerData(2));
        }
        if (mLitTime != mContainer->getContainerData(0))
        {
            listener->setContainerData(this, 0, mContainer->getContainerData(0));
        }
        if (mLitDuration != mContainer->getContainerData(1))
        {
            listener->setContainerData(this, 1, mContainer->getContainerData(1));
        }
        if (mCookingTotalTime != mContainer->getContainerData(3))
        {
            listener->setContainerData(this, 3, mContainer->getContainerData(3));
        }
    }
    mCookingProgress = mContainer->getContainerData(2);
    mLitTime = mContainer->getContainerData(0);
    mLitDuration = mContainer->getContainerData(1);
    mCookingTotalTime = mContainer->getContainerData(3);
}

void FurnaceMenu::setData(int id, int value)
{
    mContainer->setContainerData(id, value);
}

bool FurnaceMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mContainer->stillValid(player);
}

int FurnaceMenu::GetInvSlotStart()
{
    return IsSmallInventory() ? 30 : 3;
}

void FurnaceMenu::fjCheckAchievements(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item)
{
    bool fueledByCharcoal = mContainer->getContainerData(4) == 1;
    if (fueledByCharcoal && item->getItem() == Items::coal && item->getAuxValue() == 1)
    {
        player->awardStat(GenericStats::renewableEnergy(), GenericStats::param_renewableEnergy());
        return;
    }
    if (Block::byItem(item->getItem()) == Blocks::sponge)
    {
        player->awardStat(GenericStats::drySpell(), GenericStats::param_drySpell());
    }
}

not_null_ptr<ItemInstance> FurnaceMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> itemstack = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        itemstack = item->copy();
        if (index == 2)
        {
            if (!moveItemStackTo(item, GetInvSlotStart(), 39, true, true, true))
            {
                return ItemInstance::EMPTY;
            }
            slot->onQuickCraft(item, itemstack);
            fjCheckAchievements(player, itemstack);
            mContainer->setContainerData(4, 0);
        }
        else if (index == 1 || index == 0)
        {
            if (!moveItemStackTo(item, GetInvSlotStart(), 39, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (!FurnaceRecipes::getInstance()->getResult(item)->isEmpty())
        {
            if (!moveItemStackTo(item, 0, 1, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (FurnaceBlockEntity::isFuel(item))
        {
            if (!moveItemStackTo(item, 1, 2, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (index >= GetInvSlotStart() && index < 30)
        {
            if (!moveItemStackTo(item, 30, 39, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (index >= 30 && index < 39)
        {
            if (!moveItemStackTo(item, GetInvSlotStart(), 30, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }

        if (item->getCount() == 0)
        {
            slot->set(ItemInstance::EMPTY);
        }
        else
        {
            slot->setChanged();
        }
        if (item->getCount() == itemstack->getCount())
        {
            return ItemInstance::EMPTY;
        }
        else
        {
            slot->onTake(player, item);
        }
    }
    return itemstack;
}

not_null_ptr<ItemInstance> FurnaceMenu::clicked(int slot, int button, ClickType::eClickType type,
                                                boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> itemstack = AbstractContainerMenu::clicked(slot, button, type, player);
    if (slot == 2 && !itemstack->isEmpty() && (button == 0 || button == 1) && type == ClickType::eClickType_Pickup)
    {
        fjCheckAchievements(player, itemstack);
        mContainer->setContainerData(4, 0);
    }
    return itemstack;
}

void FurnaceMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    mContainer->stopOpen(player);
}
