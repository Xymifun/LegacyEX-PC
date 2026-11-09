#include "game/ui/CraftingMenu.hpp"

#include "game/core/ResultSlot.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"

const int CraftingMenu::RESULT_SLOT = 0;
const int CraftingMenu::CRAFT_SLOT_START = 1;
const int CraftingMenu::INV_SLOT_START = 10;

CraftingMenu::CraftingMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos)
    : AbstractContainerMenu(),
      m_craftSlots(),
      m_resultSlots(),
      m_playerPos()
{
    m_craftSlots = boost::shared_ptr<CraftingContainer>(new CraftingContainer(this, 3, 3));
    m_resultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    m_level = level;
    m_playerPos = pos;

    addSlot(new ResultSlot(inventory->m_player, m_craftSlots, m_resultSlots, 0, 124, 35));

    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 3; ++column)
            addSlot(new Slot(m_craftSlots, column + row * 3, 30 + column * 18, 17 + row * 18));

    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 9; ++column)
            addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));

    for (int column = 0; column < 9; ++column)
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));

    slotsChanged();
}

void CraftingMenu::slotsChanged()
{
    not_null_ptr<ItemInstance> result = Recipes::getInstance()->getItemFor(m_craftSlots, m_level, NULL,
                                                                           Recipes::eRecipeCategory_Crafting);

    int ingredientColor = -1;
    int dyeColor = -1;
    for (unsigned int slot = 0; slot < m_craftSlots->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> ingredient = m_craftSlots->getItem(slot);
        if (!ingredient->isEmpty())
        {
            if (ingredient->getItem() == result->getItem() && ingredientColor == -1)
            {
                Block* block = Block::byItem(ingredient->getItem());
                const int blockId = Block::getId(block);
                if (blockId >= BlockId::WHITE_SHULKER_BOX && blockId <= BlockId::BLACK_SHULKER_BOX)
                    ingredientColor = static_cast<ShulkerBoxBlock*>(block)->getDyeColor()->getBlockData();
                else if (ingredient->getItem() == Items::bed)
                    ingredientColor = ingredient->getAuxValue();
            }
            if (ingredient->getItem() == Items::dye)
                dyeColor = DyeColor::byItemData(ingredient->getAuxValue())->blockData;
        }
    }

    if (ingredientColor == dyeColor && ingredientColor != -1 && dyeColor != -1)
        result = ItemInstance::EMPTY;
    m_resultSlots->setItem(0, result);
}

void CraftingMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (m_level->mIsClientSide)
        return;
    clearContainer(player, m_level, m_craftSlots);
}

bool CraftingMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (m_level->getBlockState(m_playerPos)->getBlock() != Blocks::crafting_table)
        return false;
    if (player->distanceAsIfHitscanned(m_playerPos, *player->mBoundingBox)
        > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
        return false;
    return true;
}

not_null_ptr<ItemInstance> CraftingMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> clicked = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> stack = slot->getItem();
        clicked = stack->copy();

        if (index == 0)
        {
            if (!moveItemStackTo(stack, 10, 46, true, true, true))
                return ItemInstance::EMPTY;
            slot->onQuickCraft(stack, clicked);
        }
        else if (index >= 10 && index < 37)
        {
            if (!moveItemStackTo(stack, 37, 46, false, true, true))
                return ItemInstance::EMPTY;
        }
        else if (index >= 37 && index < 46)
        {
            if (!moveItemStackTo(stack, 10, 37, false, true, true))
                return ItemInstance::EMPTY;
        }
        else if (!moveItemStackTo(stack, 10, 46, false, true, true))
        {
            return ItemInstance::EMPTY;
        }

        if (stack->getCount() == 0)
            slot->set(ItemInstance::EMPTY);
        else
            slot->setChanged();

        if (stack->getCount() == clicked->getCount())
        {
            return ItemInstance::EMPTY;
        }
        else
        {
            not_null_ptr<ItemInstance> taken = slot->onTake(player, stack);
            if (index == 0)
                player->drop(taken, false);
        }
    }
    return clicked;
}

bool CraftingMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot)
{
    return slot->mContainer != m_resultSlots && AbstractContainerMenu::canTakeItemForPickAll(item, slot);
}
