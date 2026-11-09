#include "game/ui/InventoryMenu.hpp"

#include "game/core/ArmorSlot.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ResultSlot.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ArmorRecipes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/generation/Player.hpp"

const int InventoryMenu::RESULT_SLOT = 0;
const int InventoryMenu::CRAFT_SLOT_START = 1;
const int InventoryMenu::CRAFT_SLOT_END = 5;
const int InventoryMenu::ARMOR_SLOT_START = 5;
const int InventoryMenu::ARMOR_SLOT_END = 9;
const int InventoryMenu::INV_SLOT_START = 9;
const int InventoryMenu::USE_ROW_SLOT_START = 36;
const int InventoryMenu::USE_ROW_SLOT_END = 45;

const EquipmentSlot* InventoryMenu::SLOT_IDS[4];

InventoryMenu::InventoryMenu(boost::shared_ptr<Inventory> inventory, bool active, Player* player)
{
    mPlayer = player;
    _init(inventory, active);
}

void InventoryMenu::staticCtor()
{
    SLOT_IDS[3] = EquipmentSlot::FEET;
    SLOT_IDS[1] = EquipmentSlot::CHEST;
    SLOT_IDS[0] = EquipmentSlot::HEAD;
    SLOT_IDS[2] = EquipmentSlot::LEGS;
}

void InventoryMenu::_init(boost::shared_ptr<Inventory> inventory, bool active)
{
    mCraftSlots = boost::shared_ptr<CraftingContainer>(new CraftingContainer(this, 2, 2));
    mResultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    mActive = active;

    addSlot(new ResultSlot(inventory->m_player, mCraftSlots, mResultSlots, 0, 154, 28));

    int row;
    int column;
    for (row = 0; row < 2; ++row)
    {
        for (column = 0; column < 2; ++column)
        {
            addSlot(new Slot(mCraftSlots, column + row * 2, 98 + column * 18, 18 + row * 18));
        }
    }

    for (int armor = 0; armor < 4; ++armor)
    {
        addSlot(new ArmorSlot(SLOT_IDS[armor], inventory, 39 - armor, 8, 8 + armor * 18));
    }

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, 9 + row * 9 + column, 8 + column * 18, 84 + row * 18));
        }
    }

    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }

    addSlot(new OffhandSlot(inventory, 40, 77, 62));
    slotsChanged();
}

void InventoryMenu::slotsChanged()
{
    MemSect(0x17);

    not_null_ptr<ItemInstance> resultItem = Recipes::getInstance()->getItemFor(mCraftSlots, mPlayer->mLevel, NULL,
                                                                               Recipes::eRecipeCategory_Crafting);

    int craftColor = -1;
    int dyeColor = -1;
    for (unsigned int i = 0; i < mCraftSlots->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> ingredient = mCraftSlots->getItem(i);
        if (!ingredient->isEmpty())
        {
            if (ingredient->getItem() == resultItem->getItem() && craftColor == -1)
            {
                Block* block = Block::byItem(ingredient->getItem());
                const int blockId = Block::getId(block);
                if (blockId >= BlockId::WHITE_SHULKER_BOX && blockId <= BlockId::BLACK_SHULKER_BOX)
                {
                    craftColor = static_cast<ShulkerBoxBlock*>(block)->getDyeColor()->getBlockData();
                }
                else if (ingredient->getItem() == Items::bed)
                {
                    craftColor = ingredient->getAuxValue();
                }
            }
            if (ingredient->getItem() == Items::dye)
            {
                dyeColor = DyeColor::byItemData(ingredient->getAuxValue())->blockData;
            }
        }
    }

    if (craftColor == dyeColor && craftColor != -1 && dyeColor != -1)
    {
        resultItem = ItemInstance::EMPTY;
    }

    mResultSlots->setItem(0, resultItem);

    MemSect(0);
}

void InventoryMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    clearContainer(player, mPlayer->mLevel, mCraftSlots);
    mResultSlots->setItem(0, ItemInstance::EMPTY);
}

bool InventoryMenu::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

not_null_ptr<ItemInstance> InventoryMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    Slot* headSlot = m_slots.at(ARMOR_SLOT_START);
    Slot* chestSlot = m_slots.at(ARMOR_SLOT_START + 1);
    Slot* legsSlot = m_slots.at(ARMOR_SLOT_START + 2);
    Slot* feetSlot = m_slots.at(ARMOR_SLOT_START + 3);

    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        const EquipmentSlot* equipmentSlot = Mob::getEquipmentSlotForItem(result);

        bool smallInventory = Minecraft::InMiniGame(eMiniGameId_Any, true)
                              && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled();

        if (index == RESULT_SLOT)
        {
            if (!moveItemStackTo(item, INV_SLOT_START, USE_ROW_SLOT_END, true, true, true))
            {
                return ItemInstance::EMPTY;
            }
            slot->onQuickCraft(item, result);
        }
        else if (index >= CRAFT_SLOT_START && index < CRAFT_SLOT_END)
        {
            if (!moveItemStackTo(item, smallInventory ? USE_ROW_SLOT_START : INV_SLOT_START, USE_ROW_SLOT_END, false,
                                 true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (index >= ARMOR_SLOT_START && index < ARMOR_SLOT_END)
        {
            if (!moveItemStackTo(item, smallInventory ? USE_ROW_SLOT_START : INV_SLOT_START, USE_ROW_SLOT_END, false,
                                 true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (equipmentSlot->getType() == EquipmentSlot::ARMOR
                 && !m_slots.at(8 - equipmentSlot->getIndex())->hasItem())
        {
            if (!moveItemStackTo(item, 8 - equipmentSlot->getIndex(), 9 - equipmentSlot->getIndex(), false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (equipmentSlot == EquipmentSlot::OFFHAND && !m_slots.at(USE_ROW_SLOT_END)->hasItem())
        {
            if (!moveItemStackTo(item, USE_ROW_SLOT_END, USE_ROW_SLOT_END + 1, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (index >= INV_SLOT_START && index < INV_SLOT_END)
        {
            if (ArmorRecipes::GetArmorType(item->getItem()) == 1 && !headSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START, ARMOR_SLOT_START + 1, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 2 && !chestSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START + 1, ARMOR_SLOT_START + 2, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 3 && !legsSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START + 2, ARMOR_SLOT_START + 3, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 4 && !feetSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START + 3, ARMOR_SLOT_END, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (!moveItemStackTo(item, USE_ROW_SLOT_START, USE_ROW_SLOT_END, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (index >= USE_ROW_SLOT_START && index < USE_ROW_SLOT_END)
        {
            if (ArmorRecipes::GetArmorType(item->getItem()) == 1 && !headSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START, ARMOR_SLOT_START + 1, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 2 && !chestSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START + 1, ARMOR_SLOT_START + 2, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 3 && !legsSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START + 2, ARMOR_SLOT_START + 3, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 4 && !feetSlot->hasItem())
            {
                if (!moveItemStackTo(item, ARMOR_SLOT_START + 3, ARMOR_SLOT_END, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (!moveItemStackTo(item, smallInventory ? USE_ROW_SLOT_START : INV_SLOT_START, INV_SLOT_END, false,
                                      true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (!moveItemStackTo(item, smallInventory ? USE_ROW_SLOT_START : INV_SLOT_START, USE_ROW_SLOT_END, false,
                                  true, true))
        {
            return ItemInstance::EMPTY;
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
        else
        {
            not_null_ptr<ItemInstance> taken = slot->onTake(player, item);
            if (index == RESULT_SLOT)
            {
                player->drop(taken, false);
            }
        }
    }

    return result;
}

bool InventoryMenu::mayCombine(Slot* slot, not_null_ptr<ItemInstance> item)
{
    return slot->mayCombine(item);
}

bool InventoryMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot)
{
    return slot->mContainer != mResultSlots && AbstractContainerMenu::canTakeItemForPickAll(item, slot);
}

not_null_ptr<ItemInstance> InventoryMenu::clicked(int slot, int button, ClickType::eClickType type,
                                                  boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> result = AbstractContainerMenu::clicked(slot, button, type, player);

    static Item* ironArmor[4] = {Items::ironHelmet, Items::ironChestplate, Items::ironLeggings, Items::ironBoots};

    for (int armorSlot = 5; armorSlot < 9; ++armorSlot)
    {
        Slot* armorSlotObj = m_slots.at(armorSlot);
        if (armorSlotObj == NULL || !armorSlotObj->hasItem()
            || armorSlotObj->getItem()->getItem() != ironArmor[armorSlot - 5])
        {
            return result;
        }
    }

    player->awardStat(GenericStats::ironMan(), GenericStats::param_ironMan());
    return result;
}

InventoryMenu::OffhandSlot::OffhandSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool InventoryMenu::OffhandSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        Item* itemType = item->getItem();
        if (itemType == Items::filledMap || itemType == Items::arrow || itemType == Items::tippedArrow
            || itemType == Items::totemOfUndying)
        {
            return true;
        }
    }
    return false;
}
