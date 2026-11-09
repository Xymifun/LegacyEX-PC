#include "game/ui/BrewingStandMenu.hpp"

#include "game/core/GenericStats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/item/Potions.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/generation/Player.hpp"

BrewingStandMenu::PotionSlot::PotionSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool BrewingStandMenu::PotionSlot::mayPlaceItem(not_null_ptr<ItemInstance> item)
{
    Item* type = item->getItem();
    return type == Items::potion || type == Items::splashPotion || type == Items::lingeringPotion
           || type == Items::glassBottle;
}

bool BrewingStandMenu::PotionSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return mayPlaceItem(item);
}

int BrewingStandMenu::PotionSlot::getMaxStackSize()
{
    return 1;
}

not_null_ptr<ItemInstance> BrewingStandMenu::PotionSlot::onTake(boost::shared_ptr<Player> player,
                                                                not_null_ptr<ItemInstance> item)
{
    item->onCraftedBy(player->mLevel, boost::dynamic_pointer_cast<Player>(player->shared_from_this()), 1);
    Potion* potion = PotionUtils::getPotion(item);
    if (potion != Potions::WATER && potion != Potions::EMPTY)
    {
        player->awardStat(GenericStats::potion(), GenericStats::param_potion());
    }
    Slot::onTake(player, item);
    return item;
}

bool BrewingStandMenu::PotionSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}

BrewingStandMenu::IngredientsSlot::IngredientsSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool BrewingStandMenu::IngredientsSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return PotionBrewing::isIngredient(item);
}

int BrewingStandMenu::IngredientsSlot::getMaxStackSize()
{
    return 64;
}

bool BrewingStandMenu::IngredientsSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}

BrewingStandMenu::FuelSlot::FuelSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool BrewingStandMenu::FuelSlot::mayPlaceItem(not_null_ptr<ItemInstance> item)
{
    return item->getItem() == Items::blazePowder;
}

bool BrewingStandMenu::FuelSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return mayPlaceItem(item);
}

int BrewingStandMenu::FuelSlot::getMaxStackSize()
{
    return 64;
}

BrewingStandMenu::BrewingStandMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Container> container)
    : mLastBrewTime(0),
      mLastFuel(0)
{
    mContainer = container;
    addSlot(new PotionSlot(container, 0, 56, 51));
    addSlot(new PotionSlot(container, 1, 79, 58));
    addSlot(new PotionSlot(container, 2, 102, 51));
    mIngredientSlot = addSlot(new IngredientsSlot(container, 3, 79, 17));
    addSlot(new FuelSlot(container, 4, 17, 17));

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

void BrewingStandMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setAllContainerData(this, mContainer);
}

void BrewingStandMenu::broadcastChanges()
{
    AbstractContainerMenu::broadcastChanges();
    for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
         it != m_listeners.end(); ++it)
    {
        net_minecraft_world_inventory::ContainerListener* listener = *it;
        if (mLastBrewTime != mContainer->getContainerData(0))
        {
            listener->setContainerData(this, 0, mContainer->getContainerData(0));
        }
        if (mLastFuel != mContainer->getContainerData(1))
        {
            listener->setContainerData(this, 1, mContainer->getContainerData(1));
        }
    }
    mLastBrewTime = mContainer->getContainerData(0);
    mLastFuel = mContainer->getContainerData(1);
}

void BrewingStandMenu::setData(int id, int value)
{
    mContainer->setContainerData(id, value);
}

bool BrewingStandMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mContainer->stillValid(player);
}

not_null_ptr<ItemInstance> BrewingStandMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> clicked = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        clicked = item->copy();

        if ((index >= 0 && index <= 2) || index == 3 || index == 4)
        {
            if (!moveItemStackTo(item, 5, 41, true, true, true))
                return ItemInstance::EMPTY;
            slot->onQuickCraft(item, clicked);
        }
        else
        {
            if (!mIngredientSlot->hasItem() && mIngredientSlot->mayPlace(item))
            {
                if (!moveItemStackTo(item, 3, 4, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else if (PotionSlot::mayPlaceItem(item))
            {
                if (!moveItemStackTo(item, 0, 3, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else if (FuelSlot::mayPlaceItem(item))
            {
                if (!moveItemStackTo(item, 4, 5, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else if (index >= 5 && index < 32)
            {
                if (!moveItemStackTo(item, 32, 41, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else if (index >= 32 && index < 41)
            {
                if (!moveItemStackTo(item, 5, 32, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else
            {
                if (!moveItemStackTo(item, 5, 41, false, true, true))
                    return ItemInstance::EMPTY;
            }
        }

        if (item->isEmpty())
            slot->set(ItemInstance::EMPTY);
        else
            slot->setChanged();
        if (item->getCount() == clicked->getCount())
            return ItemInstance::EMPTY;
        slot->onTake(player, item);
    }
    return clicked;
}
