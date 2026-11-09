#include "game/ui/HorseInventoryMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/HorseArmorSlot.hpp"
#include "game/entity/mob/HorseSaddleSlot.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"

HorseInventoryMenu::HorseInventoryMenu(boost::shared_ptr<Container> playerInventory,
                                       boost::shared_ptr<Container> horseContainer,
                                       boost::shared_ptr<AbstractHorse> horseEntity, boost::shared_ptr<Player> player)
{
    m_container = horseContainer;
    horse = horseEntity;

    int containerRows = 3;
    horseContainer->startOpen(player);
    int yOffset = (containerRows - 4) * 18;

    addSlot(new HorseSaddleSlot(this, horseContainer, 0, 8, 18));
    addSlot(new HorseArmorSlot(this, horseContainer, 1, 8, 36));

    if (eTYPE_DERIVED_FROM(EntityType::ABSTRACT_CHESTED_HORSE, horseEntity->GetType())
        && boost::static_pointer_cast<AbstractChestedHorse>(horseEntity)->hasChest())
    {
        for (int row = 0; row < containerRows; ++row)
        {
            for (int column = 0;
                 column < boost::static_pointer_cast<AbstractChestedHorse>(horseEntity)->getInventoryColumns();
                 ++column)
            {
                addSlot(new Slot(
                    horseContainer,
                    2 + column
                        + row * boost::static_pointer_cast<AbstractChestedHorse>(horseEntity)->getInventoryColumns(),
                    80 + column * 18, 18 + row * 18));
            }
        }
    }

    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerInventory, column + row * 9 + 9, 8 + column * 18, 102 + row * 18 + yOffset));
        }
    }
    for (int column = 0; column < 9; ++column)
    {
        addSlot(new Slot(playerInventory, column, 8 + column * 18, 160 + yOffset));
    }
}

bool HorseInventoryMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_container->stillValid(player) && horse->isAlive() && horse->distanceTo(player) < 5.0f;
}

not_null_ptr<ItemInstance> HorseInventoryMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        if ((unsigned int)index < m_container->getContainerSize())
        {
            if (!moveItemStackTo(item, m_container->getContainerSize() + 27, (int)m_slots.size(), false, true, true))
            {
                if (!moveItemStackTo(item, m_container->getContainerSize(), (int)m_slots.size(), true, true, true))
                    return ItemInstance::EMPTY;
            }
        }
        else if (getSlot(1)->mayPlace(item) && !getSlot(1)->hasItem())
        {
            if (!moveItemStackTo(item, 1, 2, false, true, true))
                return ItemInstance::EMPTY;
        }
        else if (getSlot(0)->mayPlace(item))
        {
            if (!moveItemStackTo(item, 0, 1, false, true, true))
                return ItemInstance::EMPTY;
        }
        else if (m_container->getContainerSize() <= 2
                 || !moveItemStackTo(item, 2, m_container->getContainerSize(), false, true, true))
        {
            int horseSize = m_container->getContainerSize();
            int playerInventoryEnd = horseSize + 27;
            int playerSlotsEnd = playerInventoryEnd + 9;
            if (index >= horseSize && index < playerInventoryEnd)
            {
                if (!moveItemStackTo(item, playerInventoryEnd, playerSlotsEnd, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else if (index < playerSlotsEnd && index >= playerInventoryEnd)
            {
                if (!moveItemStackTo(item, horseSize, playerInventoryEnd, false, true, true))
                    return ItemInstance::EMPTY;
            }
        }

        if (item->isEmpty())
            slot->set(ItemInstance::EMPTY);
        else
            slot->setChanged();
    }
    return result;
}

void HorseInventoryMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    m_container->stopOpen(player);
}
