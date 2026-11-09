#include "game/ui/AnvilMenu.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>

#include "game/core/RepairResultSlot.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/RepairContainer.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/StringUtils.hpp"

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& position,
                     boost::shared_ptr<Player> player)
    : AbstractContainerMenu(),
      mResultSlots(),
      mInputSlots(),
      mPosition(),
      mItemName(),
      mPlayer()
{
    _init(inventory, level, position, player);
}

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level, boost::shared_ptr<Player> player)
    : AbstractContainerMenu(),
      mResultSlots(),
      mInputSlots(),
      mPosition(),
      mItemName(),
      mPlayer()
{
    _init(inventory, level, BlockPos::ZERO, player);
}

void AnvilMenu::_init(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& position,
                      boost::shared_ptr<Player> player)
{
    mResultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    mInputSlots = boost::shared_ptr<RepairContainer>(new RepairContainer(this, IDS_REPAIR_AND_NAME, true, 2));
    mRepairCost = 0;
    mMaterialCost = 0;
    mLevel = level;
    mPosition = position;
    mPlayer = player;

    addSlot(new Slot(mInputSlots, 0, 27, 47));
    addSlot(new Slot(mInputSlots, 1, 76, 47));
    addSlot(new RepairResultSlot(this, position, mResultSlots, 2, 134, 47));

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
    mItemNameCensored = false;
}

void AnvilMenu::createResult()
{
    not_null_ptr<ItemInstance> first = mInputSlots->getItem(0);
    int operationCost = 0;
    int renameCost = 0;
    mRepairCost = 1;
    if (first->isEmpty())
    {
        mResultSlots->setItem(0, ItemInstance::EMPTY);
        mRepairCost = 0;
        return;
    }
    else
    {
        not_null_ptr<ItemInstance> result = first->copy();
        not_null_ptr<ItemInstance> second = mInputSlots->getItem(1);
        boost::unordered_map<Enchantment*, int>* enchantments = EnchantmentHelper::getEnchantments(result);
        int baseCost = first->getBaseRepairCost() + (second->isEmpty() ? 0 : second->getBaseRepairCost());
        mMaterialCost = 0;

        if (!second->isEmpty())
        {
            bool enchantedBook = second->getItem() == Items::enchantedBook
                                 && EnchantedBookItem::getEnchantments(second)->size() > 0;
            if (result->isDamageableItem() && result->getItem()->isValidRepairItem(first, second))
            {
                int repairAmount = std::min(result->getDamageValue(), result->getMaxDamage() / 4);
                if (repairAmount < 1)
                {
                    mResultSlots->setItem(0, ItemInstance::EMPTY);
                    mRepairCost = 0;
                    return;
                }

                int used;
                for (used = 0; repairAmount > 0 && used < second->getCount(); ++used)
                {
                    int damage = result->getDamageValue() - repairAmount;
                    result->setAuxValue(damage);
                    ++operationCost;
                    repairAmount = std::min(result->getDamageValue(), result->getMaxDamage() / 4);
                }
                mMaterialCost = used;
            }
            else
            {
                if (!enchantedBook && (result->getItem() != second->getItem() || !result->isDamageableItem()))
                {
                    mResultSlots->setItem(0, ItemInstance::EMPTY);
                    mRepairCost = 0;
                    return;
                }

                if (result->isDamageableItem() && !enchantedBook)
                {
                    int remainingFirst = first->getMaxDamage() - first->getDamageValue();
                    int remainingSecond = second->getMaxDamage() - second->getDamageValue();
                    int bonus = remainingSecond + result->getMaxDamage() * 12 / 100;
                    int combined = remainingFirst + bonus;
                    int damage = result->getMaxDamage() - combined;
                    if (damage < 0)
                        damage = 0;
                    if (damage < result->getAuxValue())
                    {
                        result->setAuxValue(damage);
                        operationCost += 2;
                    }
                }

                boost::unordered_map<Enchantment*, int>* addedEnchantments = EnchantmentHelper::getEnchantments(second);
                bool hadEnchantment = false;
                bool appliedEnchantment = false;
                boost::unordered_map<Enchantment*, int>::iterator added;
                for (added = addedEnchantments->begin(); added != addedEnchantments->end(); ++added)
                {
                    Enchantment* enchantment = added->first;
                    if (enchantment == NULL)
                        continue;
                    boost::unordered_map<Enchantment*, int>::iterator current = enchantments->find(enchantment);
                    int oldLevel = current != enchantments->end() ? current->second : 0;
                    int level = added->second;
                    level = oldLevel == level ? level + 1 : std::max(level, oldLevel);

                    bool canApply = enchantment->canEnchant(first);
                    if (mPlayer->mAbilities.mInstabuild || first->getItem() == Items::enchantedBook)
                        canApply = true;

                    for (boost::unordered_map<Enchantment*, int>::iterator existing = enchantments->begin();
                         existing != enchantments->end(); ++existing)
                    {
                        Enchantment* other = existing->first;
                        if (other != enchantment && !enchantment->isCompatibleWith(other))
                        {
                            canApply = false;
                            ++operationCost;
                        }
                    }

                    if (!canApply)
                    {
                        hadEnchantment = true;
                    }
                    else
                    {
                        appliedEnchantment = true;
                        if (level > enchantment->getMaxLevel())
                            level = enchantment->getMaxLevel();
                        (*enchantments)[enchantment] = level;

                        int rarityCost = 0;
                        if (enchantment->getRarity()->getWeight() == Rarity::COMMON->getWeight())
                            rarityCost = 1;
                        else if (enchantment->getRarity()->getWeight() == Rarity::UNCOMMON->getWeight())
                            rarityCost = 2;
                        else if (enchantment->getRarity()->getWeight() == Rarity::RARE->getWeight())
                            rarityCost = 4;
                        else if (enchantment->getRarity()->getWeight() == Rarity::EPIC->getWeight())
                            rarityCost = 8;
                        if (enchantedBook)
                            rarityCost = std::max(1, rarityCost / 2);
                        operationCost += rarityCost * level;
                        if (first->getCount() > 1)
                            operationCost = 40;
                    }
                }
                if (hadEnchantment && !appliedEnchantment)
                {
                    mResultSlots->setItem(0, ItemInstance::EMPTY);
                    mRepairCost = 0;
                    return;
                }
                delete addedEnchantments;
            }
        }

        if (mItemName.empty())
        {
            if (first->hasCustomHoverName())
            {
                renameCost = 1;
                ++operationCost;
                result->resetHoverName();
            }
        }
        else if (!mItemName.empty() && !equalsIgnoreCase(mItemName, first->getHoverNameUnfiltered())
                 && !mItemName.empty())
        {
            renameCost = 1;
            ++operationCost;
            result->setHoverName(mItemName);
            result->setCensorshipInfo(mPlayer->getUUID(), mItemNameCensored);
        }

        mRepairCost = baseCost + operationCost;
        if (operationCost < 1)
            result = ItemInstance::EMPTY;
        if (mRepairCost >= 40)
        {
            if (renameCost == operationCost && renameCost > 0)
                mRepairCost = 39;
            else if (!mPlayer->mAbilities.mInstabuild)
                result = ItemInstance::EMPTY;
        }

        if (!result->isEmpty())
        {
            int repairCost = result->getBaseRepairCost();
            if (!second->isEmpty() && repairCost < second->getBaseRepairCost())
                repairCost = second->getBaseRepairCost();
            if (renameCost != operationCost || renameCost == 0)
                repairCost = repairCost * 2 + 1;
            result->setRepairCost(repairCost);
            EnchantmentHelper::setEnchantments(enchantments, result);
        }

        mResultSlots->setItem(0, result);
    }
    broadcastChanges();
}

void AnvilMenu::slotsChanged(boost::shared_ptr<Container> container)
{
    AbstractContainerMenu::slotsChanged();
    if (container.get() == mInputSlots.get())
        createResult();
}

void AnvilMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setContainerData(this, 0, mRepairCost);
}

void AnvilMenu::setData(int id, int value)
{
    if (id == 0)
        mRepairCost = value;
}

void AnvilMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (mLevel->mIsClientSide)
        return;
    clearContainer(player, mLevel, mInputSlots);
}

bool AnvilMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockState(mPosition)->getBlock() != Blocks::anvil)
        return false;
    if (player->distanceAsIfHitscanned(mPosition, *player->mBoundingBox)
        > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
        return false;
    return true;
}

not_null_ptr<ItemInstance> AnvilMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> clicked = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> stack = slot->getItem();
        clicked = stack->copy();
        if (index == 2)
        {
            if (!moveItemStackTo(stack, 3, 39, true, true, true))
                return ItemInstance::EMPTY;
            else
                player->OnTakeFromAnvil(clicked);
            slot->onQuickCraft(stack, clicked);
        }
        else if (index == 0 || index == 1)
        {
            if (!moveItemStackTo(stack, 3, 39, false, true, true))
                return ItemInstance::EMPTY;
        }
        else if (index >= 3 && index < 39)
        {
            if (!moveItemStackTo(stack, 0, 2, false, true, true))
                return ItemInstance::EMPTY;
        }

        if (stack->isEmpty())
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

void AnvilMenu::setItemName(const std::wstring& name, bool censored)
{
    mItemName = name;
    mItemNameCensored = censored;
    if (getSlot(2)->hasItem())
    {
        not_null_ptr<ItemInstance> result = getSlot(2)->getItem();
        if (name.empty())
            result->resetHoverName();
        else
        {
            result->setHoverName(mItemName);
            result->setCensorshipInfo(mPlayer->getUUID(), censored);
        }
    }
    createResult();
}
