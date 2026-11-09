#include "game/ui/DispenserMenu.hpp"

#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"

DispenserMenu::DispenserMenu(boost::shared_ptr<Container> playerInventory, boost::shared_ptr<Container> dispenser)
{
    m_dispenser = dispenser;

    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            addSlot(new Slot(dispenser, column + row * 3, 62 + column * 18, 17 + row * 18));
        }
    }

    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerInventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
        }
    }

    for (int column = 0; column < 9; ++column)
    {
        addSlot(new Slot(playerInventory, column, 8 + column * 18, 142));
    }
}

DispenserMenu::~DispenserMenu()
{
}

bool DispenserMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_dispenser->stillValid(player);
}

not_null_ptr<ItemInstance> DispenserMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);

    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        bool smallInventory = Minecraft::InMiniGame(eMiniGameId_Any, true)
                              && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled();

        if (index < 9)
        {
            if (!moveItemStackTo(item, smallInventory ? 36 : 9, 45, true, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else
        {
            if (!moveItemStackTo(item, 0, 9, false, true, true))
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

        if (item->getCount() == result->getCount())
        {
            return ItemInstance::EMPTY;
        }
        else
        {
            slot->onTake(player, item);
        }
    }

    return result;
}
