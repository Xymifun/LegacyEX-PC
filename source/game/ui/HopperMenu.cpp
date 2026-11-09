#include "game/ui/HopperMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"

HopperMenu::HopperMenu(boost::shared_ptr<Container> playerContainer, boost::shared_ptr<Container> container,
                       boost::shared_ptr<Player> player)
    : AbstractContainerMenu()
{
    m_container = container;
    container->startOpen(player);

    int row;
    int column;
    for (unsigned int i = 0; i < container->getContainerSize(); ++i)
    {
        addSlot(new Slot(container, i, 44 + i * 18, 20));
    }

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerContainer, column + row * 9 + 9, 8 + column * 18, 51 + row * 18));
        }
    }

    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(playerContainer, column, 8 + column * 18, 109));
    }
}

bool HopperMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_container->stillValid(player);
}

not_null_ptr<ItemInstance> HopperMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        if ((unsigned int)index < m_container->getContainerSize())
        {
            if (!moveItemStackTo(item, m_container->getContainerSize() + 27, m_slots.size(), false, true, true))
            {
                if (!moveItemStackTo(item, m_container->getContainerSize(), m_slots.size(), true, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
        }
        else if (!moveItemStackTo(item, 0, m_container->getContainerSize(), false, true, true))
        {
            return ItemInstance::EMPTY;
        }
        if (item->getCount() == 0)
        {
            slot->set(ItemInstance::EMPTY);
        }
        else
        {
            slot->setChanged();
        }
    }
    return result;
}

void HopperMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    m_container->stopOpen(player);
}
