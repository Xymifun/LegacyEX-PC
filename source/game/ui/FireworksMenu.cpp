#include "game/ui/FireworksMenu.hpp"

#include "game/core/ResultSlot.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

FireworksMenu::FireworksMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos)
    : AbstractContainerMenu(),
      m_craftSlots(),
      m_resultSlots(),
      m_playerPos(),
      m_hasPaper(false),
      m_hasGunpowder(false),
      m_hasFireworkCharge(false)
{
    m_craftSlots = boost::shared_ptr<CraftingContainer>(new CraftingContainer(this, 3, 3));
    m_resultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    m_level = level;
    m_playerPos = pos;

    addSlot(new ResultSlot(inventory->m_player, m_craftSlots, m_resultSlots, 0, 124, 35));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 3; ++column)
            addSlot(new Slot(m_craftSlots, row * 3 + column, 30 + column * 18, 17 + row * 18));
    }

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
            addSlot(new Slot(inventory, 9 + row * 9 + column, 8 + column * 18, 84 + row * 18));
    }

    for (column = 0; column < 9; ++column)
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));

    slotsChanged();
}

void FireworksMenu::slotsChanged()
{
    FireworksRecipe::updatePossibleRecipes(m_craftSlots, &m_hasPaper, &m_hasGunpowder, &m_hasFireworkCharge);
    m_resultSlots->setItem(0, Recipes::getInstance()->getItemFor(m_craftSlots, m_level, Recipes::mFireworksRecipe,
                                                                 Recipes::eRecipeCategory_Crafting));
}

void FireworksMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (m_level->mIsClientSide)
        return;
    clearContainer(player, m_level, m_craftSlots);
}

bool FireworksMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (m_level->getBlockState(m_playerPos)->getBlock() != Blocks::crafting_table)
        return false;
    if (player->distanceAsIfHitscanned(m_playerPos, *player->mBoundingBox)
        > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
        return false;
    return true;
}

not_null_ptr<ItemInstance> FireworksMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
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
            if (isValidIngredient(stack, -1) && moveItemStackTo(stack, 1, 10, false, true, true))
            {
            }
            else if (!moveItemStackTo(stack, 37, 46, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (index >= 37 && index < 46)
        {
            if (isValidIngredient(stack, -1) && moveItemStackTo(stack, 1, 10, false, true, true))
            {
            }
            else if (!moveItemStackTo(stack, 10, 37, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
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
            return ItemInstance::EMPTY;
        else
            slot->onTake(player, stack);
    }
    return clicked;
}

bool FireworksMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot)
{
    return slot->mContainer != m_resultSlots && AbstractContainerMenu::canTakeItemForPickAll(item, slot);
}

bool FireworksMenu::isValidIngredient(not_null_ptr<ItemInstance> item, int slot)
{
    if (item->isEmpty() || slot == 0)
        return true;
    return FireworksRecipe::isValidIngredient(item, m_hasPaper, m_hasGunpowder, m_hasFireworkCharge);
}
