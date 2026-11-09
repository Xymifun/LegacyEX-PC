#include "game/ui/EnchantmentMenu.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentCostSlot.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/EnchantmentSlot.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/Level.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

EnchantmentMenu::EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos)
    : AbstractContainerMenu()
{
    mEnchantmentContainer = boost::shared_ptr<EnchantmentContainer>(new EnchantmentContainer(this));
    for (int i = 0; i < 3; ++i)
        mCosts[i] = 0;
    for (int i = 0; i < 3; ++i)
    {
        mEnchantmentIds[i] = -1;
        mEnchantmentLevels[i] = -1;
    }
    mLevel = level;
    mPosition = pos;
    mEnchantmentSeed = static_cast<long long>(static_cast<int>(inventory->m_player->getEnchantmentSeed()));

    addSlot(new EnchantmentSlot(mEnchantmentContainer, 0, 15, 47));
    addSlot(new EnchantmentCostSlot(mEnchantmentContainer, 1, 35, 47));

    for (int row = 0; row < 3; ++row)
    {
        for (int column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, row * 9 + column + 9, 8 + column * 18, 84 + row * 18));
        }
    }
    for (int column = 0; column < 9; ++column)
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));

    mDataChanged = false;
}

EnchantmentMenu::EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level) : AbstractContainerMenu()
{
    EnchantmentMenu(inventory, level, BlockPos::ZERO);
}

void EnchantmentMenu::broadcastData(net_minecraft_world_inventory::ContainerListener* listener)
{
    listener->setContainerData(this, 0, mCosts[0]);
    listener->setContainerData(this, 1, mCosts[1]);
    listener->setContainerData(this, 2, mCosts[2]);
    listener->setContainerData(this, 3, static_cast<int>(mEnchantmentSeed & -16));
    listener->setContainerData(this, 4, mEnchantmentIds[0]);
    listener->setContainerData(this, 5, mEnchantmentIds[1]);
    listener->setContainerData(this, 6, mEnchantmentIds[2]);
    listener->setContainerData(this, 7, mEnchantmentLevels[0]);
    listener->setContainerData(this, 8, mEnchantmentLevels[1]);
    listener->setContainerData(this, 9, mEnchantmentLevels[2]);
}

void EnchantmentMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    broadcastData(listener);
}

void EnchantmentMenu::broadcastChanges()
{
    AbstractContainerMenu::broadcastChanges();

    if (mDataChanged)
    {
        for (unsigned int i = 0; i < m_listeners.size(); ++i)
            broadcastData(m_listeners.at(i));
        mDataChanged = false;
    }
}

void EnchantmentMenu::setData(int id, int value)
{
    if (static_cast<unsigned int>(id) < 3)
    {
        mCosts[id] = value;
        mDataChanged = true;
    }
    else if (id == 3)
    {
        mEnchantmentSeed = static_cast<long long>(value);
    }
    else if (static_cast<unsigned int>(id - 4) < 3)
    {
        mEnchantmentIds[id - 4] = value;
    }
    else if (static_cast<unsigned int>(id - 7) < 3)
    {
        mEnchantmentLevels[id - 7] = value;
    }
    else
    {
        AbstractContainerMenu::setData(id, value);
    }
}

std::vector<EnchantmentInstance*>* EnchantmentMenu::getEnchantmentList(not_null_ptr<ItemInstance> item, int enchantSlot,
                                                                       int cost)
{
    mRandom.setSeed(mEnchantmentSeed + static_cast<long long>(enchantSlot));
    std::vector<EnchantmentInstance*>* results = EnchantmentHelper::selectEnchantment(&mRandom, item, cost, false);
    if (results != NULL && item->getItem() == Items::book && results->size() > 1)
        results->erase(results->begin() + mRandom.nextInt(static_cast<int>(results->size())));
    return results;
}

void EnchantmentMenu::slotsChanged()
{
    not_null_ptr<ItemInstance> item = mEnchantmentContainer->getItem(0);
    if (item->isEmpty() || !item->isEnchantable())
    {
        for (int slot = 0; slot < 3; ++slot)
        {
            mCosts[slot] = 0;
            mEnchantmentIds[slot] = -1;
            mEnchantmentLevels[slot] = -1;
        }
        mDataChanged = true;
    }
    else if (!mLevel->mIsClientSide)
    {
        int bookshelves = 0;
        for (int dz = -1; dz <= 1; ++dz)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if ((dz != 0 || dx != 0) && mLevel->isEmptyBlock(mPosition.offset(dx, 0, dz))
                    && mLevel->isEmptyBlock(mPosition.offset(dx, 1, dz)))
                {
                    if (mLevel->getBlockState(mPosition.offset(dx * 2, 0, dz * 2))->getBlock() == Blocks::bookshelf)
                        ++bookshelves;
                    if (mLevel->getBlockState(mPosition.offset(dx * 2, 1, dz * 2))->getBlock() == Blocks::bookshelf)
                        ++bookshelves;
                    if (dx != 0 && dz != 0)
                    {
                        if (mLevel->getBlockState(mPosition.offset(dx * 2, 0, dz))->getBlock() == Blocks::bookshelf)
                            ++bookshelves;
                        if (mLevel->getBlockState(mPosition.offset(dx * 2, 1, dz))->getBlock() == Blocks::bookshelf)
                            ++bookshelves;
                        if (mLevel->getBlockState(mPosition.offset(dx, 0, dz * 2))->getBlock() == Blocks::bookshelf)
                            ++bookshelves;
                        if (mLevel->getBlockState(mPosition.offset(dx, 1, dz * 2))->getBlock() == Blocks::bookshelf)
                            ++bookshelves;
                    }
                }
            }
        }

        mRandom.setSeed(mEnchantmentSeed);
        for (int slot = 0; slot < 3; ++slot)
        {
            int cost = EnchantmentHelper::getEnchantmentCost(&mRandom, slot, bookshelves, item);
            mEnchantmentIds[slot] = -1;
            mEnchantmentLevels[slot] = -1;
            if (cost < slot + 1)
                cost = 0;
            mCosts[slot] = cost;
        }

        for (int slot = 0; slot < 3; ++slot)
        {
            if (mCosts[slot] > 0)
            {
                std::vector<EnchantmentInstance*>* results = getEnchantmentList(item, slot, mCosts[slot]);
                if (results != NULL)
                {
                    if (!results->empty())
                    {
                        EnchantmentInstance* instance = (*results)[mRandom.nextInt(static_cast<int>(results->size()))];
                        mEnchantmentIds[slot] = Enchantment::getId(instance->enchantment);
                        mEnchantmentLevels[slot] = instance->level;
                    }
                    while (!results->empty())
                    {
                        delete results->back();
                        results->pop_back();
                    }
                    delete results;
                }
            }
        }
        mDataChanged = true;
        broadcastChanges();
    }
}

bool EnchantmentMenu::clickMenuButton(boost::shared_ptr<Player> player, int id)
{
    not_null_ptr<ItemInstance> item = mEnchantmentContainer->getItem(0);
    not_null_ptr<ItemInstance> lapis = mEnchantmentContainer->getItem(1);
    const int lapisRequired = id + 1;

    if ((lapis->isEmpty() || lapis->getCount() < lapisRequired) && !player->mAbilities.mInstabuild)
        return false;

    if (mCosts[id] > 0 && !item->isEmpty()
        && ((player->mExperienceLevel >= lapisRequired && player->mExperienceLevel >= mCosts[id])
            || player->mAbilities.mInstabuild))
    {
        if (!mLevel->mIsClientSide)
        {
            std::vector<EnchantmentInstance*>* results = getEnchantmentList(item, id, mCosts[id]);
            if (!results->empty())
            {
                player->onEnchantmentPerformed(lapisRequired);

                bool isBook = item->getItem() == Items::book;
                if (isBook)
                {
                    item = not_null_ptr<ItemInstance>(new ItemInstance(Items::enchantedBook));
                    mEnchantmentContainer->setItem(0, item);
                }
                EnchantmentInstance* instance;
                unsigned int i = 0;
                if (isBook)
                {
                    for (; i < results->size(); ++i)
                    {
                        instance = results->at(i);
                        EnchantedBookItem::addEnchantment(item, instance);
                        if (instance != 0)
                            delete instance;
                    }
                }
                else
                {
                    for (; i < results->size(); ++i)
                    {
                        instance = results->at(i);
                        item->enchant(instance->enchantment, instance->level);
                        delete instance;
                    }
                }
                delete results;

                if (!player->mAbilities.mInstabuild)
                {
                    lapis->shrink(lapisRequired);
                    if (lapis->isEmpty())
                        mEnchantmentContainer->setItem(1, ItemInstance::EMPTY);
                }

                mEnchantmentContainer->setChanged();
                mEnchantmentSeed = static_cast<long long>(static_cast<int>(player->getEnchantmentSeed()));
                slotsChanged();
                mLevel->playSound(boost::shared_ptr<Player>(), mPosition, SoundEvents::ENCHANTMENT_TABLE_USE,
                                  SoundSource::eSoundSource_Blocks, 1.0f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f,
                                  16.0f);
            }
            else
            {
                delete results;
            }
        }
        return true;
    }
    return false;
}

int EnchantmentMenu::getGoldCount()
{
    not_null_ptr<ItemInstance> item = mEnchantmentContainer->getItem(1);
    if (item->isEmpty())
        return 0;
    return item->getCount();
}

void EnchantmentMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (mLevel->mIsClientSide)
        return;
    clearContainer(player, mLevel, mEnchantmentContainer);
}

bool EnchantmentMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockState(mPosition)->getBlock() != Blocks::enchanting_table)
        return false;
    if (player->distanceAsIfHitscanned(mPosition, *player->mBoundingBox)
        > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
        return false;
    return true;
}

not_null_ptr<ItemInstance> EnchantmentMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> clicked = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    Slot* itemSlot = m_slots.at(0);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> stack = slot->getItem();
        clicked = stack->copy();

        if (index == 0 || index == 1)
        {
            if (!moveItemStackTo(stack, 2, 29, true, true, true))
            {
                if (!moveItemStackTo(stack, 29, 38, false, true, true))
                    return ItemInstance::EMPTY;
            }
        }
        else if (stack->getItem() == Items::dye && DyeColor::byItemData(stack->getAuxValue()) == DyeColor::BLUE)
        {
            if (!moveItemStackTo(stack, 1, 2, true, true, true))
                return ItemInstance::EMPTY;
        }
        else if (index >= 2 && index < 29)
        {
            if (stack->isEnchantable() && !itemSlot->hasItem())
            {
                if (!moveItemStackTo(stack, 0, 1, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else
            {
                if (!moveItemStackTo(stack, 29, 38, false, true, true))
                    return ItemInstance::EMPTY;
            }
        }
        else if (index >= 29 && index < 38)
        {
            if (stack->isEnchantable() && !itemSlot->hasItem())
            {
                if (!moveItemStackTo(stack, 0, 1, false, true, true))
                    return ItemInstance::EMPTY;
            }
            else
            {
                if (!moveItemStackTo(stack, 2, 29, false, true, true))
                    return ItemInstance::EMPTY;
            }
        }
        else
        {
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
