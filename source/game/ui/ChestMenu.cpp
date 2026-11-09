#include "game/ui/ChestMenu.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Slot.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

static const int CHESTFUL_OF_COBBLESTONE = 27 * 64;

ChestMenu::ChestMenu(boost::shared_ptr<Container> playerInventory, boost::shared_ptr<Container> container,
                     boost::shared_ptr<Player> player)
{
    m_container = container;
    m_numRows = container->getContainerSize() / 9;
    container->startOpen(player);

    int yo = (m_numRows - 4) * 18;
    int row;
    int column;

    for (row = 0; row < m_numRows; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(container, column + row * 9, 8 + column * 18, 18 + row * 18));
        }
    }
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerInventory, column + row * 9 + 9, 8 + column * 18, 103 + row * 18 + yo));
        }
    }
    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(playerInventory, column, 8 + column * 18, 161 + yo));
    }
}

bool ChestMenu::stillValid(boost::shared_ptr<Player> player)
{
    return m_container->stillValid(player);
}

not_null_ptr<ItemInstance> ChestMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();

        if (index < m_numRows * 9)
        {
            if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            {
                bool replacedExisting = false;
                if (player->AutoEquip(result, item, replacedExisting))
                {
                    slot->set(ItemInstance::EMPTY);
                    return ItemInstance::EMPTY;
                }

                bool small = IsSmallInventory();
                int containerEnd = m_numRows * 9;
                int hotbarStart = containerEnd + 27;
                int slotCount = (int)m_slots.size();
                bool moved;
                if (small)
                {
                    moved = moveItemStackTo(item, hotbarStart, slotCount, false, true, true);
                }
                else
                {
                    moved = moveItemStackTo(item, containerEnd, slotCount, true, true, false);
                    if (!moved)
                    {
                        if (item->getItem()->PreferInventory())
                        {
                            moved = moveItemStackTo(item, containerEnd, hotbarStart, true, false, true);
                            if (!moved)
                                moved = moveItemStackTo(item, hotbarStart, slotCount, false, false, true);
                        }
                        else
                        {
                            moved = moveItemStackTo(item, hotbarStart, slotCount, false, false, true);
                            if (!moved)
                                moved = moveItemStackTo(item, containerEnd, hotbarStart, true, false, true);
                        }
                    }
                }

                if (replacedExisting)
                {
                    if (moved)
                        slot->set(ItemInstance::EMPTY);
                    else
                        slot->set(item);
                }
                else if (slot->getItem()->getCount() == 0)
                {
                    slot->set(ItemInstance::EMPTY);
                }
                return ItemInstance::EMPTY;
            }
            else if (!moveItemStackTo(item, m_numRows * 9, (int)m_slots.size(), true, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (!moveItemStackTo(item, 0, m_numRows * 9, false, true, true))
        {
            return ItemInstance::EMPTY;
        }

        if (item->getCount() == 0)
            slot->set(ItemInstance::EMPTY);
        else
            slot->setChanged();
    }
    return result;
}

void ChestMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    m_container->stopOpen(player);
}

boost::shared_ptr<Container> ChestMenu::getContainer()
{
    return m_container;
}

not_null_ptr<ItemInstance> ChestMenu::clicked(int slot, int button, ClickType::eClickType type,
                                              boost::shared_ptr<Player> player)
{
    bool carriedIsCobblestone = Block::byItem(player->mInventory->getCarried()->getItem()) == Blocks::cobblestone;

    bool clickedCobblestone = type == ClickType::eClickType_QuickMove && slot != -999 && m_slots[slot] != NULL
                              && Block::byItem(m_slots[slot]->getItem()->getItem()) == Blocks::cobblestone;

    not_null_ptr<ItemInstance> result = AbstractContainerMenu::clicked(slot, button, type, player);

    boost::shared_ptr<LocalPlayer> localPlayer = boost::dynamic_pointer_cast<LocalPlayer>(player);
    if (localPlayer && (carriedIsCobblestone || clickedCobblestone))
    {
        int cobblestoneCount = 0;
        unsigned int i;
        for (i = 0; i < m_container->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> containerItem = m_container->getItem(i);
            if (Block::byItem(containerItem->getItem()) == Blocks::cobblestone)
                cobblestoneCount += containerItem->getCount();
        }

        StatsCounter* stats = Minecraft::GetInstance()->mStats[localPlayer->mUserIndex];
        Stat* stoneStat = gStats->get_blocksMined(Block::getId(Blocks::stone));
        Stat* cobblestoneStat = gStats->get_blocksMined(Block::getId(Blocks::cobblestone));
        int stoneMined = stats->getTotalValue(stoneStat);
        int cobblestoneMined = stats->getTotalValue(cobblestoneStat);

        if (cobblestoneCount >= CHESTFUL_OF_COBBLESTONE && stoneMined + cobblestoneMined >= CHESTFUL_OF_COBBLESTONE)
        {
            localPlayer->awardStat(GenericStats::chestfulOfCobblestone(),
                                   GenericStats::param_chestfulOfCobblestone(cobblestoneCount));
        }
    }

    return result;
}
