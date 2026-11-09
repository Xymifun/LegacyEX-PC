#include "game/ui/AbstractContainerMenu.hpp"
#include "game/core/EntityType.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/minigame/MiniGameDef.hpp"

#include <algorithm>

AbstractContainerMenu::AbstractContainerMenu()
{
    m_containerId = 0;
    m_transactionId = 0;
    m_quickcraftType = -1;
    m_quickcraftStatus = 0;
    m_needsRendered = false;
    m_uiQuickCraftActive = false;
    m_uiQuickCraftRemaining = 0;
}

AbstractContainerMenu::~AbstractContainerMenu()
{
    unsigned int i;
    for (i = 0; i < m_slots.size(); ++i)
    {
        delete m_slots.at(i);
    }
}

Slot* AbstractContainerMenu::addSlot(Slot* slot)
{
    slot->mIndex = (int)m_slots.size();
    m_slots.push_back(slot);
    m_items.push_back(ItemInstance::EMPTY);
    return slot;
}

void AbstractContainerMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    m_listeners.push_back(listener);
    ItemList* items = getItems();
    listener->refreshContainer(this, items);
    delete items;
    broadcastChanges();
}

void AbstractContainerMenu::removeSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = std::find(
        m_listeners.begin(), m_listeners.end(), listener);
    if (it != m_listeners.end())
        m_listeners.erase(it);
}

AbstractContainerMenu::ItemList* AbstractContainerMenu::getItems()
{
    ItemList* items = new ItemList();
    std::vector<Slot*>::iterator end = m_slots.end();
    for (std::vector<Slot*>::iterator it = m_slots.begin(); it != end; ++it)
        items->push_back((*it)->getItem());
    return items;
}

void AbstractContainerMenu::sendData(int id, int value)
{
    std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator end = m_listeners.end();
    for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin(); it != end;
         ++it)
        (*it)->setContainerData(this, id, value);
}

void AbstractContainerMenu::broadcastChanges()
{
    unsigned int i;
    for (i = 0; i < m_slots.size(); ++i)
    {
        not_null_ptr<ItemInstance> current = m_slots.at(i)->getItem();
        not_null_ptr<ItemInstance> stored = m_items.at(i);
        if (!ItemInstance::matches(stored, current, true))
        {
            stored = current->isEmpty() ? ItemInstance::EMPTY : current->copy();
            m_items[i] = stored;
            m_needsRendered = true;
            std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator end = m_listeners.end();
            for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
                 it != end; ++it)
                (*it)->slotChanged(this, (int)i, stored);
        }
    }
}

bool AbstractContainerMenu::needsRendered()
{
    bool changed = m_needsRendered;
    m_needsRendered = false;
    unsigned int i;
    for (i = 0; i < m_slots.size(); ++i)
    {
        not_null_ptr<ItemInstance> current = m_slots.at(i)->getItem();
        not_null_ptr<ItemInstance> stored = m_items.at(i);
        if (!ItemInstance::matches(stored, current, true))
        {
            stored = current->isEmpty() ? ItemInstance::EMPTY : current->copy();
            m_items[i] = stored;
            changed = true;
        }
    }
    return changed;
}
bool AbstractContainerMenu::clickMenuButton(boost::shared_ptr<Player>, int)
{
    return false;
}

Slot* AbstractContainerMenu::getSlotFor(boost::shared_ptr<Container> container, int index)
{
    std::vector<Slot*>::iterator end = m_slots.end();
    for (std::vector<Slot*>::iterator it = m_slots.begin(); it != end; ++it)
    {
        Slot* slot = *it;
        if (slot->isAt(container, index))
        {
            return slot;
        }
    }
    return NULL;
}

Slot* AbstractContainerMenu::getSlot(int index)
{
    if (index < (int)m_slots.size())
        return m_slots.at(index);
    return NULL;
}

AbstractContainerMenu::ItemList AbstractContainerMenu::getAllItems(int start, int count)
{
    ItemList items;
    items.reserve(count);
    int i;
    for (i = start; i < start + count; ++i)
        items.push_back(m_slots.at(i)->getItem());
    return items;
}

AbstractContainerMenu::ItemList AbstractContainerMenu::takeAll(boost::shared_ptr<Player> player, int button, int start,
                                                               int count)
{
    ItemList items;
    items.reserve(count);
    int i;
    for (i = start; i < start + count; ++i)
        items.push_back(clicked(i, button, ClickType::eClickType_QuickMove, player));
    return items;
}

not_null_ptr<ItemInstance> AbstractContainerMenu::quickMoveStack(boost::shared_ptr<Player>, int index)
{
    Slot* slot = m_slots.at(index);
    if (slot != NULL)
        return slot->getItem();
    return ItemInstance::EMPTY;
}

int AbstractContainerMenu::getQuickcraftHeader(int flags)
{
    return flags & 3;
}

void AbstractContainerMenu::resetQuickCraft()
{
    m_quickcraftStatus = 0;
    m_quickcraftSlots.clear();
}
int AbstractContainerMenu::getQuickcraftType(int flags)
{
    return (flags >> 2) & 3;
}
bool AbstractContainerMenu::isValidQuickcraftType(int type, boost::shared_ptr<Player> player)
{
    if (type == 0 || type == 1)
        return true;
    if (type == 2 && player->mAbilities.mInstabuild)
        return true;
    return false;
}

bool AbstractContainerMenu::canItemQuickReplace(Slot* slot, not_null_ptr<ItemInstance> item, bool stacking)
{
    bool slotEmpty = slot == NULL || !slot->hasItem();
    if (slot != NULL && slot->hasItem() && !item->isEmpty() && item->sameItem(slot->getItem())
        && ItemInstance::tagMatches(slot->getItem(), item))
    {
        slotEmpty |= slot->getItem()->getCount() + (stacking ? 0 : item->getCount()) <= item->getMaxStackSize();
    }
    return slotEmpty;
}

bool AbstractContainerMenu::canDragTo(Slot*)
{
    return true;
}

void AbstractContainerMenu::getQuickCraftSlotCount(SlotSet* slots, int mode, not_null_ptr<ItemInstance> item,
                                                   int slotCount)
{
    switch (mode)
    {
    case 0:
        item->setCount(Mth::floor((float)item->getCount() / (float)slots->size()));
        break;
    case 1:
        item->setCount(1);
        break;
    case 2:
        item->setCount(item->getItem()->getMaxStackSize());
        break;
    }
    item->grow(slotCount);
}

not_null_ptr<ItemInstance> AbstractContainerMenu::clicked(int index, int button, ClickType::eClickType type,
                                                          boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    boost::shared_ptr<Inventory> inventory = player->mInventory;
    int previousStatus = m_quickcraftStatus;

    if (type == ClickType::eClickType_QuickCraft)
    {
        m_quickcraftStatus = getQuickcraftHeader(button);
        if ((previousStatus == 1 && m_quickcraftStatus == 2) || previousStatus == m_quickcraftStatus)
        {
            if (!inventory->getCarried()->isEmpty())
            {
                if (m_quickcraftStatus == 0)
                {
                    m_quickcraftType = getQuickcraftType(button);
                    if (isValidQuickcraftType(m_quickcraftType, player))
                    {
                        m_quickcraftStatus = 1;
                        m_quickcraftSlots.clear();
                    }
                    else
                    {
                        resetQuickCraft();
                    }
                }
                else if (m_quickcraftStatus == 1)
                {
                    Slot* slot = m_slots.at(index);
                    not_null_ptr<ItemInstance> carried = inventory->getCarried();
                    if (slot != NULL && canItemQuickReplace(slot, carried, true) && slot->mayPlace(carried)
                        && (m_quickcraftType == 2 || carried->getCount() > (int)m_quickcraftSlots.size())
                        && canDragTo(slot))
                    {
                        m_quickcraftSlots.insert(slot);
                    }
                }
                else if (m_quickcraftStatus == 2)
                {
                    if (!m_quickcraftSlots.empty())
                    {
                        not_null_ptr<ItemInstance> dragged = inventory->getCarried()->copy();
                        int remaining = inventory->getCarried()->getCount();
                        SlotSet::iterator it;
                        for (it = m_quickcraftSlots.begin(); it != m_quickcraftSlots.end(); ++it)
                        {
                            Slot* slot = *it;
                            not_null_ptr<ItemInstance> carried = inventory->getCarried();
                            if (slot != NULL && canItemQuickReplace(slot, carried, true) && slot->mayPlace(carried)
                                && carried->getCount() >= (int)m_quickcraftSlots.size() && canDragTo(slot))
                            {
                                not_null_ptr<ItemInstance> placed = dragged->copy();
                                int alreadyThere = slot->hasItem() ? slot->getItem()->getCount() : 0;
                                getQuickCraftSlotCount(&m_quickcraftSlots, m_quickcraftType, placed, alreadyThere);
                                int limit = placed->getMaxStackSize();
                                int slotLimit = slot->getMaxStackSize(placed);
                                if (slotLimit < limit)
                                    limit = slotLimit;
                                if (limit < placed->getCount())
                                    placed->setCount(limit);
                                remaining -= placed->getCount() - alreadyThere;
                                slot->set(placed);
                            }
                        }
                        dragged->setCount(remaining);
                        inventory->setCarried(dragged);
                    }
                    resetQuickCraft();
                }
                else
                {
                    resetQuickCraft();
                }
            }
            else
            {
                resetQuickCraft();
            }
        }
        else
        {
            resetQuickCraft();
        }
    }
    else if (m_quickcraftStatus != 0)
    {
        resetQuickCraft();
    }
    else if ((type == ClickType::eClickType_Pickup || type == ClickType::eClickType_QuickMove)
             && (button == 0 || button == 1))
    {
        if (index == -999)
        {
            if (!inventory->getCarried()->isEmpty())
            {
                if (button == 0)
                {
                    player->drop(inventory->getCarried(), true);
                    inventory->setCarried(ItemInstance::EMPTY);
                }
                if (button == 1)
                {
                    player->drop(inventory->getCarried()->split(1), true);
                }
            }
        }
        else if (type == ClickType::eClickType_QuickMove)
        {
            if (index < 0)
                return ItemInstance::EMPTY;
            Slot* slot = m_slots.at(index);
            if (slot == NULL || !slot->mayPickup(player))
                return ItemInstance::EMPTY;
            not_null_ptr<ItemInstance> moved = quickMoveStack(player, index);
            while (!moved->isEmpty() && ItemInstance::isSame(slot->getItem(), moved) && doQuickMoveLoop())
            {
                result = moved->copy();
                moved = quickMoveStack(player, index);
            }
        }
        else
        {
            if (index < 0)
                return ItemInstance::EMPTY;
            Slot* slot = m_slots.at(index);
            if (slot != NULL)
            {
                not_null_ptr<ItemInstance> slotItem = slot->getItem();
                not_null_ptr<ItemInstance> carried = inventory->getCarried();
                if (!slotItem->isEmpty())
                    result = slotItem->copy();
                if (!slotItem->isEmpty())
                {
                    bool combining = false;
                    if (button == 1 && mayCombine(slot, carried))
                        combining = true;
                    if (combining)
                    {
                        not_null_ptr<ItemInstance> combined = slot->combine(carried);
                        if (!combined->isEmpty())
                        {
                            slot->set(combined);
                            combined->onCraftedBy(player->mLevel,
                                                  boost::static_pointer_cast<Player>(player->shared_from_this()), 1);
                            if (!player->mAbilities.mInstabuild)
                                carried->split(1);
                        }
                    }
                    else if (slot->mayPickup(player))
                    {
                        if (!carried->isEmpty())
                        {
                            if (!slot->mayPlace(carried))
                            {
                                if (slotItem->getItem() == carried->getItem() && carried->getMaxStackSize() > 1
                                    && (!slotItem->isStackedByData()
                                        || slotItem->getAuxValue() == carried->getAuxValue())
                                    && ItemInstance::tagMatches(slotItem, carried))
                                {
                                    int taken = slotItem->getCount();
                                    if (taken + carried->getCount() <= carried->getMaxStackSize())
                                    {
                                        carried->grow(taken);
                                        slotItem = slot->remove(taken);
                                        if (slotItem->isEmpty())
                                            slot->set(ItemInstance::EMPTY);
                                        slot->onTake(player, inventory->getCarried());
                                    }
                                }
                            }
                            else if (slotItem->getItem() == carried->getItem()
                                     && slotItem->getAuxValue() == carried->getAuxValue()
                                     && ItemInstance::tagMatches(slotItem, carried))
                            {
                                int amount = button == 0 ? carried->getCount() : 1;
                                if (slot->getMaxStackSize(carried) - slotItem->getCount() < amount)
                                    amount = slot->getMaxStackSize(carried) - slotItem->getCount();
                                if (carried->getMaxStackSize() - slotItem->getCount() < amount)
                                    amount = carried->getMaxStackSize() - slotItem->getCount();
                                carried->shrink(amount);
                                slotItem->grow(amount);
                            }
                            else if (carried->getCount() <= slot->getMaxStackSize(carried))
                            {
                                slot->set(carried);
                                inventory->setCarried(slotItem);
                            }
                        }
                        else if (!slotItem->isEmpty())
                        {
                            int amount = button == 0 ? slotItem->getCount() : (slotItem->getCount() + 1) / 2;
                            inventory->setCarried(slot->remove(amount));
                            if (slotItem->isEmpty())
                                slot->set(ItemInstance::EMPTY);
                            slot->onTake(player, inventory->getCarried());
                        }
                        else
                        {
                            slot->set(ItemInstance::EMPTY);
                            inventory->setCarried(ItemInstance::EMPTY);
                        }
                    }
                }
                else if (!carried->isEmpty() && slot->mayPlace(carried))
                {
                    int amount = button == 0 ? carried->getCount() : 1;
                    if (slot->getMaxStackSize(carried) < amount)
                        amount = slot->getMaxStackSize(carried);
                    slot->set(carried->split(amount));
                }
                slot->setChanged();
            }
        }
    }
    else if (type == ClickType::eClickType_Swap && button >= 0 && button < 9)
    {
        Slot* slot = m_slots.at(index);
        not_null_ptr<ItemInstance> hotbarItem = inventory->getItem(button);
        not_null_ptr<ItemInstance> slotItem = slot->getItem();
        if (!hotbarItem->isEmpty() || !slotItem->isEmpty())
        {
            if (hotbarItem->isEmpty())
            {
                if (slot->mayPickup(player))
                {
                    inventory->setItem(button, slotItem);
                    slot->onSwapCraft(slotItem->getCount());
                    slot->set(ItemInstance::EMPTY);
                    slot->onTake(player, slotItem);
                }
            }
            else if (slotItem->isEmpty())
            {
                if (slot->mayPlace(hotbarItem))
                {
                    int limit = slot->getMaxStackSize(hotbarItem);
                    if (hotbarItem->getCount() > limit)
                    {
                        slot->set(hotbarItem->split(limit));
                    }
                    else
                    {
                        slot->set(hotbarItem);
                        inventory->setItem(button, ItemInstance::EMPTY);
                    }
                }
            }
            else if (slot->mayPickup(player) && slot->mayPlace(hotbarItem))
            {
                int limit = slot->getMaxStackSize(hotbarItem);
                if (hotbarItem->getCount() > limit)
                {
                    slot->set(hotbarItem->split(limit));
                    slot->onTake(player, slotItem);
                    if (!inventory->add(slotItem))
                        player->drop(slotItem, true);
                }
                else
                {
                    slot->set(hotbarItem);
                    inventory->setItem(button, slotItem);
                    slot->onTake(player, slotItem);
                }
            }
        }
    }
    else if (type == ClickType::eClickType_Clone && player->mAbilities.mInstabuild && inventory->getCarried()->isEmpty()
             && index >= 0)
    {
        Slot* slot = m_slots.at(index);
        if (slot != NULL && slot->hasItem())
        {
            not_null_ptr<ItemInstance> cloned = slot->getItem()->copy();
            cloned->setCount(cloned->getMaxStackSize());
            inventory->setCarried(cloned);
            slot->set(ItemInstance::EMPTY);
        }
    }
    else if (type == ClickType::eClickType_Throw && inventory->getCarried()->isEmpty() && index >= 0)
    {
        Slot* slot = m_slots.at(index);
        if (slot != NULL && slot->hasItem() && slot->mayPickup(player))
        {
            not_null_ptr<ItemInstance> thrown = slot->remove(button == 0 ? 1 : slot->getItem()->getCount());
            slot->onTake(player, thrown);
            player->drop(thrown, true);
        }
    }
    else if (type == ClickType::eClickType_PickupAll && index >= 0)
    {
        Slot* slot = m_slots.at(index);
        not_null_ptr<ItemInstance> carried = inventory->getCarried();
        if (!carried->isEmpty() && (slot == NULL || !slot->hasItem() || !slot->mayPickup(player)))
        {
            int start = button == 0 ? 0 : (int)m_slots.size() - 1;
            int step = button == 0 ? 1 : -1;
            int pass;
            for (pass = 0; pass < 2; ++pass)
            {
                int at;
                for (at = start;
                     at >= 0 && at < (int)m_slots.size() && carried->getCount() < carried->getMaxStackSize();
                     at += step)
                {
                    Slot* source = m_slots.at(at);
                    if (source->hasItem() && canItemQuickReplace(source, carried, true) && source->mayPickup(player)
                        && canTakeItemForPickAll(carried, source))
                    {
                        not_null_ptr<ItemInstance> sourceItem = source->getItem();
                        if (pass != 0 || sourceItem->getCount() != sourceItem->getMaxStackSize())
                        {
                            int amount = carried->getMaxStackSize() - carried->getCount();
                            if (sourceItem->getCount() < amount)
                                amount = sourceItem->getCount();
                            not_null_ptr<ItemInstance> taken = source->remove(amount);
                            carried->grow(amount);
                            if (taken->isEmpty())
                                source->set(ItemInstance::EMPTY);
                            source->onTake(player, taken);
                        }
                    }
                }
            }
        }
        broadcastChanges();
    }

    return result;
}

bool AbstractContainerMenu::mayCombine(Slot*, not_null_ptr<ItemInstance>)
{
    return false;
}
bool AbstractContainerMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance>, Slot*)
{
    return true;
}

void AbstractContainerMenu::clearContainer(boost::shared_ptr<Player> player, Level* level,
                                           boost::shared_ptr<Container> container)
{
    boost::shared_ptr<ServerPlayer> serverPlayer;
    if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, player->GetType()))
        serverPlayer = boost::static_pointer_cast<ServerPlayer>(player);
    if (!player->isAlive()
        || (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, player->GetType()) && serverPlayer->hasDisconnected()))
    {
        unsigned int i;
        for (i = 0; i < container->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> item = container->removeItemNoUpdate(i);
            if (!item->isEmpty())
                player->drop(item, false);
        }
    }
    else
    {
        unsigned int i;
        for (i = 0; i < container->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> item = container->removeItemNoUpdate(i);
            if (!item->isEmpty())
                player->mInventory->placeItemBackInInventory(level, item);
        }
    }
}

void AbstractContainerMenu::removed(boost::shared_ptr<Player> player)
{
    boost::shared_ptr<Inventory> inventory = player->mInventory;
    if (!inventory->getCarried()->isEmpty())
    {
        player->drop(inventory->getCarried(), false);
        inventory->setCarried(ItemInstance::EMPTY);
    }
}
void AbstractContainerMenu::slotsChanged()
{
    broadcastChanges();
}

void AbstractContainerMenu::setItem(unsigned int index, not_null_ptr<ItemInstance> item)
{
    getSlot(index)->set(item);
}

void AbstractContainerMenu::setAll(NonNullList<not_null_ptr<ItemInstance> > items)
{
    unsigned int i;
    for (i = 0; i < items.size(); ++i)
        getSlot(i)->set(items[i]);
}

void AbstractContainerMenu::setData(int, int)
{
}
int AbstractContainerMenu::backup(boost::shared_ptr<Inventory>)
{
    return ++m_transactionId;
}
bool AbstractContainerMenu::isSynched(boost::shared_ptr<Player> player)
{
    return m_synched.find(player) == m_synched.end();
}
void AbstractContainerMenu::setSynched(boost::shared_ptr<Player> player, bool value)
{
    if (value)
    {
        PlayerSet::iterator it = m_synched.find(player);
        if (it != m_synched.end())
            m_synched.erase(it);
    }
    else
    {
        m_synched.insert(player);
    }
}
bool AbstractContainerMenu::stackAsdAsd(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second)
{
    return second->getItem() == first->getItem()
           && (!first->isStackedByData() || first->getAuxValue() == second->getAuxValue())
           && ItemInstance::tagMatches(first, second);
}

bool AbstractContainerMenu::moveItemStackTo(not_null_ptr<ItemInstance> item, int start, int end, bool reverse,
                                            bool mergeStacks, bool fillEmpty)
{
    bool moved = false;
    if (mergeStacks && item->isStackable())
    {
        int i = reverse ? end - 1 : start;
        while (item->getCount() > 0 && (!reverse && i < end || reverse && i >= start))
        {
            Slot* slot = m_slots.at(i);
            not_null_ptr<ItemInstance> slotItem = slot->getItem();
            if (!slotItem->isEmpty() && stackAsdAsd(item, slotItem))
            {
                int limit = slot->getMaxStackSize();
                if (item->getMaxStackSize() < limit)
                    limit = item->getMaxStackSize();
                int total = slotItem->getCount() + item->getCount();
                if (total <= limit)
                {
                    item->setCount(0);
                    slotItem->setCount(total);
                    slot->setChanged();
                    moved = true;
                }
                else if (slotItem->getCount() < limit)
                {
                    item->shrink(limit - slotItem->getCount());
                    slotItem->setCount(limit);
                    slot->setChanged();
                    moved = true;
                }
            }
            if (reverse)
                --i;
            else
                ++i;
        }
    }
    if ((fillEmpty || moved) && item->getCount() > 0)
    {
        int i = reverse ? end - 1 : start;
        while (!reverse && i < end || reverse && i >= start)
        {
            Slot* slot = m_slots.at(i);
            not_null_ptr<ItemInstance> slotItem = slot->getItem();
            if (slotItem->isEmpty() && slot->mayPlace(item))
            {
                not_null_ptr<ItemInstance> placed = item->copy();
                int limit = slot->getMaxStackSize(placed);
                if (limit < placed->getCount())
                {
                    placed->setCount(limit);
                    item->shrink(limit);
                }
                else
                {
                    item->setCount(0);
                }
                slot->set(placed);
                slot->setChanged();
                moved = true;
                break;
            }
            if (reverse)
                --i;
            else
                ++i;
        }
    }
    return moved;
}

bool AbstractContainerMenu::isOverrideResultClick(int, int)
{
    return false;
}
int AbstractContainerMenu::getQuickcraftMask(int header, int type)
{
    return (header & 3) | ((type & 3) << 2);
}

int AbstractContainerMenu::getRedstoneSignalFromContainer(boost::shared_ptr<Container> container)
{
    if (container.get() == NULL)
        return 0;
    int filled = 0;
    float fill = 0.0f;
    unsigned int i;
    for (i = 0; i < container->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = container->getItem(i);
        if (!item->isEmpty())
        {
            int limit = container->getMaxStackSize();
            if (item->getMaxStackSize() < limit)
                fill += (float)item->getCount() / (float)item->getMaxStackSize();
            else
                fill += (float)item->getCount() / (float)limit;
            ++filled;
        }
    }
    fill = fill / (float)container->getContainerSize();
    return Mth::floor(fill * 14.0f) + (filled > 0 ? 1 : 0);
}

int AbstractContainerMenu::getRedstoneSignalFromBlockEntity(boost::shared_ptr<BlockEntity> entity)
{
    if (eTYPE_DERIVED_FROM(BlockEntityType::CONTAINER, entity->GetType()))
        return getRedstoneSignalFromContainer(boost::dynamic_pointer_cast<Container>(entity));
    return 0;
}
bool AbstractContainerMenu::isValidIngredient(not_null_ptr<ItemInstance>, int)
{
    return true;
}
not_null_ptr<ItemInstance> AbstractContainerMenu::GetQuickCraftItem(int playerIndex, int slotIndex)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    not_null_ptr<ItemInstance> preview = ItemInstance::EMPTY;
    if (m_uiQuickCraftActive)
    {
        bool useCarried = false;
        if (m_uiQuickCraftSlots.size() != 0)
        {
            not_null_ptr<ItemInstance> carried = minecraft->mLocalPlayers[playerIndex]->mInventory->getCarried();
            if (!carried->isEmpty() && slotIndex == -1)
                useCarried = true;
        }
        if (useCarried)
        {
            preview = minecraft->mLocalPlayers[playerIndex]->mInventory->getCarried()->copy();
            preview->setCount(m_uiQuickCraftRemaining);
            preview->mUnknown = 1;
        }
        else if (slotIndex >= 0)
        {
            Slot* slot = getSlot(slotIndex);
            bool spread = false;
            if (m_uiQuickCraftSlots.find(slot) != m_uiQuickCraftSlots.end())
            {
                not_null_ptr<ItemInstance> carried = minecraft->mLocalPlayers[playerIndex]->mInventory->getCarried();
                if (!carried->isEmpty())
                    spread = true;
            }
            if (spread)
            {
                bool placeable = false;
                if (canItemQuickReplace(slot, minecraft->mLocalPlayers[playerIndex]->mInventory->getCarried(), true)
                    && canDragTo(slot))
                {
                    placeable = true;
                }
                if (placeable)
                {
                    preview = minecraft->mLocalPlayers[playerIndex]->mInventory->getCarried()->copy();
                    getQuickCraftSlotCount(&m_uiQuickCraftSlots, 0, preview, slot->getItem()->getCount());
                    if (preview->getCount() > preview->getMaxStackSize())
                        preview->setCount(preview->getMaxStackSize());
                    if (preview->getCount() > slot->getMaxStackSize(preview))
                        preview->setCount(slot->getMaxStackSize(preview));
                }
            }
        }
    }
    return preview;
}
bool AbstractContainerMenu::IsSmallInventory()
{
    bool small = false;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame)
        && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
    {
        small = true;
    }
    return small;
}
bool AbstractContainerMenu::matches(AbstractContainerMenu* first, AbstractContainerMenu* second)
{
    if (first == NULL || second == NULL)
        return false;
    if (first->m_slots.size() != second->m_slots.size())
        return false;
    for (unsigned int index = 0; index < first->m_slots.size(); ++index)
    {
        not_null_ptr<ItemInstance> firstItem = first->m_slots.at(index)->getItem();
        not_null_ptr<ItemInstance> secondItem = second->m_slots.at(index)->getItem();
        if (!ItemInstance::matches(firstItem, secondItem, true))
        {
            return false;
        }
    }
    return true;
}
