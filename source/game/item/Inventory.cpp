#include "game/item/Inventory.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <vector>

#include "game/core/GenericStats.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/GameMode.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/network/ClientboundContainerSetSlotPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"

const int Inventory::INVENTORY_SIZE = 36;

Inventory::Inventory(Player* player) : m_auxiliaryItem(ItemInstance::EMPTY), m_carried(ItemInstance::EMPTY)
{
    m_items = NonNullList<not_null_ptr<ItemInstance> >::withSize(INVENTORY_SIZE, ItemInstance::EMPTY);
    m_armor = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    m_offhand = NonNullList<not_null_ptr<ItemInstance> >::withSize(1, ItemInstance::EMPTY);
    m_compartments[0] = m_items;
    m_compartments[1] = m_armor;
    m_compartments[2] = m_offhand;
    m_hasPickedItem = false;
    m_selected = 0;
    m_carried = ItemInstance::EMPTY;
    m_player = player;
    m_timesChanged = 0;
}

Inventory::~Inventory()
{
    delete[] m_items.items.data;
    delete[] m_armor.items.data;
    delete[] m_offhand.items.data;
}

std::wstring Inventory::getName()
{
    return std::wstring(L10N::GetString(IDS_INVENTORY));
}

std::wstring Inventory::getCustomName()
{
    return std::wstring(L"");
}

bool Inventory::hasCustomName()
{
    return false;
}

std::wstring Inventory::getDisplayName()
{
    return hasCustomName() ? getCustomName() : getName();
}

bool Inventory::isHotbarSlot(int slot)
{
    return slot >= 0 && slot < 9;
}

const not_null_ptr<ItemInstance>& Inventory::getSelected()
{
    return isHotbarSlot(m_selected) ? m_items[m_selected] : ItemInstance::EMPTY;
}

int Inventory::getSelectionSize()
{
    return 9;
}

bool Inventory::isSameItem(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second, bool ignoreTags)
{
    return first->getItem() == second->getItem()
           && (!first->isStackedByData() || first->getAuxValue() == second->getAuxValue())
           && (ignoreTags || ItemInstance::tagMatches(first, second));
}

bool Inventory::hasRemainingSpaceForItem(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second,
                                         bool ignoreTags)
{
    return !first->isEmpty() && isSameItem(first, second, ignoreTags) && first->isStackable()
           && first->getCount() < first->getMaxStackSize() && first->getCount() < getMaxStackSize();
}

int Inventory::getFreeSlot()
{
    unsigned int size = m_items.size();
    if (Minecraft::InMiniGame(eMiniGameId_Any, true)
        && Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
    {
        size = 9;
    }
    for (unsigned int slot = 0; slot < size; ++slot)
    {
        if (m_items[slot]->isEmpty())
        {
            return static_cast<int>(slot);
        }
    }
    return -1;
}

bool Inventory::hasPickedItem()
{
    bool result = m_hasPickedItem;
    m_hasPickedItem = false;
    return result;
}

int Inventory::findSlotMatchingItem(not_null_ptr<ItemInstance> item, bool ignoreTags)
{
    for (int slot = 0; slot < static_cast<int>(m_items.size()); ++slot)
    {
        if (!m_items[slot]->isEmpty() && isSameItem(item, m_items[slot], ignoreTags))
        {
            return slot;
        }
    }
    return -1;
}

int Inventory::getSuitableHotbarSlot()
{
    for (unsigned int offset = 0; offset < 9; ++offset)
    {
        int slot = (m_selected + offset) % 9;
        if (m_items[slot]->isEmpty())
        {
            return slot;
        }
    }
    for (unsigned int offset = 0; offset < 9; ++offset)
    {
        int slot = (m_selected + offset) % 9;
        if (!m_items[slot]->isEnchanted())
        {
            return slot;
        }
    }
    return m_selected;
}

void Inventory::pickSlot(int slot)
{
    int selected = getSuitableHotbarSlot();
    m_selected = selected;
    not_null_ptr<ItemInstance> selectedItem = m_items[selected];
    m_items[selected] = m_items[slot];
    m_items[slot] = selectedItem;
}

void Inventory::setPickedItem(not_null_ptr<ItemInstance> item)
{
    int slot = findSlotMatchingItem(item, false);
    if (isHotbarSlot(slot))
    {
        m_selected = slot;
        m_hasPickedItem = true;
    }
    else if (slot != -1)
    {
        pickSlot(slot);
    }
    else if (m_player->mAbilities.mInstabuild)
    {
        m_selected = getSuitableHotbarSlot();
        if (!m_items[m_selected]->isEmpty())
        {
            int freeSlot = getFreeSlot();
            if (freeSlot != -1)
            {
                m_items[freeSlot] = m_items[m_selected];
            }
        }
        m_items[m_selected] = item;
    }
}

int Inventory::findSlotMatchingUnusedItem(not_null_ptr<ItemInstance> item)
{
    for (int slot = 0; slot < static_cast<int>(m_items.size()); ++slot)
    {
        not_null_ptr<ItemInstance> current = m_items[slot];
        if (!m_items[slot]->isEmpty() && isSameItem(item, m_items[slot], true) && !m_items[slot]->isDamaged()
            && !current->isEnchanted() && !current->hasCustomHoverName())
        {
            return slot;
        }
    }
    return -1;
}

void Inventory::swapPaint(int direction)
{
    if (direction > 0)
    {
        direction = 1;
    }
    else if (direction < 0)
    {
        direction = -1;
    }
    m_selected -= direction;
    while (m_selected < 0)
    {
        m_selected += 9;
    }
    while (m_selected >= 9)
    {
        m_selected -= 9;
    }
}

int Inventory::clearInventory(Item* item, int auxValue, int count, CompoundTag* tag)
{
    int removed = 0;
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> current = getItem(slot);
        if (current->isEmpty())
        {
            continue;
        }
        if (item != NULL && current->getItem() != item)
        {
            continue;
        }
        if (auxValue >= 0 && current->getAuxValue() != auxValue)
        {
            continue;
        }
        int amount = count < 1 ? current->getCount() : std::min(current->getCount(), count - removed);
        removed += amount;
        if (count != 0)
        {
            m_items[slot]->shrink(amount);
            if (m_items[slot]->isEmpty())
            {
                setItem(slot, ItemInstance::EMPTY);
            }
            if (count > 0 && removed >= count)
            {
                return removed;
            }
        }
    }

    if (!m_carried->isEmpty())
    {
        if (item == NULL && m_carried->getItem() != NULL)
        {
            return removed;
        }
        if (auxValue >= 0 && m_carried->getAuxValue() != auxValue)
        {
            return removed;
        }
        int amount = count < 1 ? m_carried->getCount() : std::min(m_carried->getCount(), count - removed);
        removed += amount;
        if (count != 0)
        {
            m_carried->shrink(amount);
            if (m_carried->isEmpty())
            {
                m_carried = ItemInstance::EMPTY;
            }
        }
    }
    return removed;
}

int Inventory::getSlotWithRemainingSpace(not_null_ptr<ItemInstance> item)
{
    if (hasRemainingSpaceForItem(getItem(m_selected), item, false))
    {
        return m_selected;
    }
    if (hasRemainingSpaceForItem(getItem(40), item, false))
    {
        return 40;
    }
    for (int slot = 0; slot < static_cast<int>(m_items.size()); ++slot)
    {
        if (hasRemainingSpaceForItem(m_items[slot], item, false))
        {
            return slot;
        }
    }
    return -1;
}

int Inventory::addResource(int slot, not_null_ptr<ItemInstance> item)
{
    Item* itemType = item->getItem();
    int remaining = item->getCount();
    not_null_ptr<ItemInstance> current = getItem(slot);
    if (current->isEmpty())
    {
        current = not_null_ptr<ItemInstance>(new ItemInstance(itemType, 0, item->getAuxValue()));
        if (item->hasTag())
        {
            current->setTag(item->getTag()->copy());
            m_player->handleCollectItem(item);
        }
        setItem(slot, current);
    }

    int amount = remaining;
    if (amount > current->getMaxStackSize() - current->getCount())
    {
        amount = current->getMaxStackSize() - current->getCount();
    }
    if (amount > getMaxStackSize() - current->getCount())
    {
        amount = getMaxStackSize() - current->getCount();
    }
    if (amount == 0)
    {
        return remaining;
    }
    remaining -= amount;
    current->grow(amount);
    current->setPopTime(5);
    return remaining;
}

int Inventory::addResource(not_null_ptr<ItemInstance> item)
{
    GameMode* gameMode = m_player->GetGameMode();
    if (gameMode != NULL)
    {
        gameMode->RecordItem(item);
    }
    int slot = getSlotWithRemainingSpace(item);
    if (slot == -1)
    {
        slot = getFreeSlot();
    }
    return slot == -1 ? item->getCount() : addResource(slot, item);
}

void Inventory::tick()
{
    NonNullList<not_null_ptr<ItemInstance> >* compartment = m_compartments;
    for (int remaining = 3; remaining != 0; --remaining, ++compartment)
    {
        for (unsigned int slot = 0; slot < compartment->size(); ++slot)
        {
            if (!(*compartment)[slot]->isEmpty())
            {
                (*compartment)[slot]->inventoryTick(m_player->mLevel, m_player->shared_from_this(), slot,
                                                    m_selected == static_cast<int>(slot));
            }
        }
    }
}

bool Inventory::add(int slot, not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        return false;
    }

    not_null_ptr<ItemInstance> original = item->copy();
    if (!item->isDamaged())
    {
        int count;
        if (slot == -1)
        {
            do
            {
                count = item->getCount();
                item->setCount(addResource(item));
            } while (!item->isEmpty() && item->getCount() < count);
        }
        else
        {
            do
            {
                count = item->getCount();
                item->setCount(addResource(slot, item));
            } while (!item->isEmpty() && item->getCount() < count);
        }
        if (item->getCount() == count && m_player->mAbilities.mInstabuild)
        {
            item->setCount(0);
            return true;
        }
        if (item->getCount() < count)
        {
            m_player->awardStat(
                GenericStats::itemsCollected(original),
                GenericStats::param_itemsCollected(original->getItem(), original->getAuxValue(), original->getCount()));
            if (original->getItem() == Items::leather)
            {
                m_player->awardStat(GenericStats::killCow(), GenericStats::param_noArgs());
            }
            return true;
        }
        return false;
    }
    if (slot == -1)
    {
        slot = getFreeSlot();
    }
    if (slot >= 0)
    {
        m_player->handleCollectItem(item);
        m_player->awardStat(GenericStats::itemsCollected(item), GenericStats::param_itemsCollected(item));
        if (original->getItem() == Items::leather)
        {
            m_player->awardStat(GenericStats::killCow(), GenericStats::param_noArgs());
        }
        m_items[slot] = item->copy();
        m_items[slot]->setPopTime(5);
        item->setCount(0);
        return true;
    }
    if (m_player->mAbilities.mInstabuild)
    {
        item->setCount(0);
        return true;
    }
    return false;
}

bool Inventory::add(not_null_ptr<ItemInstance> item)
{
    return add(-1, item);
}

void Inventory::placeItemBackInInventory(Level* level, not_null_ptr<ItemInstance> item)
{
    if (level->mIsClientSide)
    {
        return;
    }

    while (!item->isEmpty())
    {
        int slot = getSlotWithRemainingSpace(item);
        if (slot == -1)
        {
            slot = getFreeSlot();
        }
        if (slot == -1)
        {
            m_player->drop(item, false);
            break;
        }

        int space = item->getMaxStackSize() - getItem(slot)->getCount();
        if (add(slot, item->split(space)))
        {
            static_cast<ServerPlayer*>(m_player)->mConnection->send(
                boost::shared_ptr<Packet>(new ClientboundContainerSetSlotPacket(-2, slot, getItem(slot))));
        }
    }
}

not_null_ptr<ItemInstance> Inventory::removeItem(unsigned int slot, int count)
{
    NonNullList<not_null_ptr<ItemInstance> > compartment;
    for (int index = 0; index < 3; ++index)
    {
        if (slot < m_compartments[index].size())
        {
            compartment = m_compartments[index];
            break;
        }
        slot -= m_compartments[index].size();
    }
    if (!compartment[slot]->isEmpty())
    {
        return ContainerHelper::removeItem(compartment, slot, count);
    }
    return ItemInstance::EMPTY;
}

void Inventory::removeItem(not_null_ptr<ItemInstance> item)
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        for (int slot = 0; slot < static_cast<int>(m_compartments[compartment].size()); ++slot)
        {
            if (m_compartments[compartment][slot].get() == item.get())
            {
                m_compartments[compartment][slot] = ItemInstance::EMPTY;
                break;
            }
        }
    }
}

not_null_ptr<ItemInstance> Inventory::removeItemNoUpdate(int slot)
{
    NonNullList<not_null_ptr<ItemInstance> > compartment;
    for (int index = 0; index < 3; ++index)
    {
        if (slot < m_compartments[index].size())
        {
            compartment = m_compartments[index];
            break;
        }
        slot -= m_compartments[index].size();
    }
    if (compartment[slot]->isEmpty())
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> item = compartment[slot];
    compartment[slot] = ItemInstance::EMPTY;
    return item;
}

void Inventory::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    NonNullList<not_null_ptr<ItemInstance> > compartment;
    for (int index = 0; index < 3; ++index)
    {
        if (slot < m_compartments[index].size())
        {
            if (index == 1 && !item->isEmpty() && eTYPE_DERIVED_FROM(EntityType::LOCAL_PLAYER, m_player->GetType()))
            {
                m_player->OnEquipArmor(item);
            }
            compartment = m_compartments[index];
            break;
        }
        slot -= m_compartments[index].size();
    }
    compartment[slot] = item;
    m_player->handleCollectItem(item);
}

float Inventory::getDestroySpeed(const BlockState* block)
{
    float speed = 1.0f;
    if (!m_items[m_selected]->isEmpty())
    {
        speed *= m_items[m_selected]->getDestroySpeed(block);
    }
    return speed;
}

ListTag* Inventory::save(ListTag* tag)
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (!m_items[slot]->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", static_cast<unsigned char>(slot));
            m_items[slot]->save(itemTag);
            tag->add(itemTag);
        }
    }
    for (unsigned int slot = 0; slot < m_armor.size(); ++slot)
    {
        if (!m_armor[slot]->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", static_cast<unsigned char>(slot + 100));
            m_armor[slot]->save(itemTag);
            tag->add(itemTag);
        }
    }
    for (unsigned int slot = 0; slot < m_offhand.size(); ++slot)
    {
        if (!m_offhand[slot]->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", static_cast<unsigned char>(slot + 150));
            m_offhand[slot]->save(itemTag);
            tag->add(itemTag);
        }
    }
    return tag;
}

void Inventory::load(ListTag* tag)
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        m_items[slot] = m_items.defaultValue;
    }
    for (unsigned int slot = 0; slot < m_armor.size(); ++slot)
    {
        m_armor[slot] = m_armor.defaultValue;
    }
    for (unsigned int slot = 0; slot < m_offhand.size(); ++slot)
    {
        m_offhand[slot] = m_offhand.defaultValue;
    }
    for (int index = 0; index < tag->size(); ++index)
    {
        CompoundTag* itemTag = tag->getCompound(index);
        unsigned int slot = itemTag->getByte(L"Slot");
        not_null_ptr<ItemInstance> item(new ItemInstance(itemTag));
        if (!item->isEmpty())
        {
            if (slot < m_items.size())
            {
                m_items[slot] = item;
            }
            else if (slot >= 100)
            {
                if (slot < 100 + m_armor.size())
                {
                    m_armor[slot - 100] = item;
                }
                else if (slot >= 150 && slot < 150 + m_offhand.size())
                {
                    m_offhand[slot - 150] = item;
                }
            }
        }
    }
}

unsigned int Inventory::getContainerSize()
{
    return m_items.size() + m_armor.size() + m_offhand.size();
}

bool Inventory::isEmpty()
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = m_items[slot];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    for (unsigned int slot = 0; slot < m_armor.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = m_armor[slot];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    for (unsigned int slot = 0; slot < m_offhand.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = m_offhand[slot];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> Inventory::getItem(unsigned int slot)
{
    NonNullList<not_null_ptr<ItemInstance> > compartment;
    for (int index = 0; index < 3; ++index)
    {
        if (slot < m_compartments[index].size())
        {
            compartment = m_compartments[index];
            break;
        }
        slot -= m_compartments[index].size();
    }
    return compartment.size() == 0 ? ItemInstance::EMPTY : compartment[slot];
}

int Inventory::getMaxStackSize()
{
    return 64;
}

bool Inventory::canDestroy(const BlockState* block)
{
    if (block->getMaterial()->isAlwaysDestroyable())
    {
        return true;
    }
    not_null_ptr<ItemInstance> selected = getItem(m_selected);
    if (!selected->isEmpty())
    {
        return selected->canDestroySpecial(block);
    }
    return false;
}

not_null_ptr<ItemInstance> Inventory::getArmor(int slot)
{
    return m_armor[slot];
}

void Inventory::hurtArmor(float damage)
{
    damage /= 4.0f;
    if (damage < 1.0f)
    {
        damage = 1.0f;
    }
    for (unsigned int slot = 0; slot < m_armor.size(); ++slot)
    {
        if (!m_armor[slot]->isEmpty() && dynamic_cast<ArmorItem*>(m_armor[slot]->getItem()) != NULL)
        {
            m_armor[slot]->hurtAndBreak(static_cast<int>(damage),
                                        boost::static_pointer_cast<LivingEntity>(m_player->shared_from_this()));
            if (m_armor[slot]->getCount() == 0)
            {
                m_armor[slot] = ItemInstance::EMPTY;
            }
        }
    }
}

void Inventory::dropAll()
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (!m_items[slot]->isEmpty())
        {
            m_player->drop(m_items[slot], true, false);
            m_items[slot] = ItemInstance::EMPTY;
        }
    }
    for (unsigned int slot = 0; slot < m_armor.size(); ++slot)
    {
        if (!m_armor[slot]->isEmpty())
        {
            m_player->drop(m_armor[slot], true, false);
            m_armor[slot] = ItemInstance::EMPTY;
        }
    }
    for (unsigned int slot = 0; slot < m_offhand.size(); ++slot)
    {
        if (!m_offhand[slot]->isEmpty())
        {
            m_player->drop(m_offhand[slot], true, false);
            m_offhand[slot] = ItemInstance::EMPTY;
        }
    }
}

void Inventory::setChanged()
{
    ++m_timesChanged;
}

void Inventory::setCarried(not_null_ptr<ItemInstance> item)
{
    m_carried = item;
    m_player->handleCollectItem(item);
}

not_null_ptr<ItemInstance> Inventory::getCarried()
{
    return m_carried;
}

bool Inventory::stillValid(boost::shared_ptr<Player> player)
{
    if (m_player->mRemoved)
    {
        return false;
    }
    if (player->distanceAsIfHitscanned(BlockPos(m_player->getPos(1.0f)), *player->mBoundingBox)
        > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
    {
        return false;
    }
    return true;
}

void Inventory::startOpen(boost::shared_ptr<Player> player)
{
}

void Inventory::stopOpen(boost::shared_ptr<Player> player)
{
}

bool Inventory::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

void Inventory::replaceWith(boost::shared_ptr<Inventory> inventory)
{
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        setItem(slot, inventory->getItem(slot));
    }
    m_selected = inventory->m_selected;
}

void Inventory::replaceWith_NONRECURSIVE(boost::shared_ptr<Inventory> inventory)
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        unsigned int size = std::min(m_compartments[compartment].size(), inventory->m_compartments[compartment].size());
        for (unsigned int slot = 0; slot < size; ++slot)
        {
            m_compartments[compartment][slot] = inventory->m_compartments[compartment][slot]->copy();
        }
    }
}

int Inventory::getContainerData(int id)
{
    return 0;
}

void Inventory::setContainerData(int id, int value)
{
}

int Inventory::getContainerDataCount()
{
    return 0;
}

void Inventory::clearContent()
{
    for (int compartment = 0; compartment < 3; ++compartment)
    {
        m_compartments[compartment].clear();
    }
}

int Inventory::countMatches(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        return 0;
    }
    int count = 0;
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (m_items[slot]->sameItem(item))
        {
            count += m_items[slot]->getCount();
        }
    }
    return count;
}

void Inventory::OnRemove(int slot)
{
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || slot < 0 || slot >= 9)
    {
        return;
    }
    for (unsigned int source = 9; source < INVENTORY_SIZE; ++source)
    {
        if (!m_items[source]->isEmpty() && !m_items[source]->getItem()->PreferInventory())
        {
            m_items[slot] = m_items[source];
            m_items[source] = ItemInstance::EMPTY;
            return;
        }
    }
}

void Inventory::PrintContents()
{
    for (unsigned int slot = 0; slot < m_items.size(); ++slot)
    {
        if (!m_items[slot]->isEmpty())
        {
            m_items[slot]->toString();
        }
    }
}

int Inventory::getSlot(Item* item, int auxValue, CompoundTag* tag)
{
    std::vector<std::wstring>* exceptions = new std::vector<std::wstring>();
    exceptions->push_back(L"display");
    exceptions->push_back(L"Name");
    for (unsigned int slot = 0; slot < m_compartments[0].size(); ++slot)
    {
        if (m_compartments[0][slot]->getItem() == item
            && (auxValue < 0 || m_compartments[0][slot]->getAuxValue() == auxValue))
        {
            if (tag == NULL)
            {
                return static_cast<int>(slot);
            }
            if (m_compartments[0][slot]->getTag() != NULL
                && tag->equalsWithException(m_compartments[0][slot]->getTag(), exceptions))
            {
                return static_cast<int>(slot);
            }
        }
    }
    for (unsigned int slot = 0; slot < m_compartments[1].size(); ++slot)
    {
        if (m_compartments[1][slot]->getItem() == item
            && (auxValue < 0 || m_compartments[1][slot]->getAuxValue() == auxValue))
        {
            if (tag == NULL)
            {
                return static_cast<int>(slot);
            }
            if (m_compartments[1][slot]->getTag() != NULL
                && tag->equalsWithException(m_compartments[1][slot]->getTag(), exceptions))
            {
                return static_cast<int>(slot);
            }
        }
    }
    for (unsigned int slot = 0; slot < m_compartments[2].size(); ++slot)
    {
        if (m_compartments[2][slot]->getItem() == item
            && (auxValue < 0 || m_compartments[2][slot]->getAuxValue() == auxValue))
        {
            if (tag == NULL)
            {
                return static_cast<int>(slot);
            }
            if (m_compartments[2][slot]->getTag() != NULL
                && tag->equalsWithException(m_compartments[2][slot]->getTag(), exceptions))
            {
                return static_cast<int>(slot);
            }
        }
    }
    return -1;
}

bool Inventory::removeResource(Item* item, bool flag)
{
    int slot = getSlot(item, -1, NULL);
    if (slot < 0)
    {
        return false;
    }
    removeItem(slot, 1);
    return true;
}

bool Inventory::removeResource(Item* item, int auxValue, bool flag)
{
    int slot = getSlot(item, auxValue, NULL);
    if (slot < 0)
    {
        return false;
    }
    removeItem(slot, 1);
    return true;
}

void Inventory::removeResources(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        return;
    }
    int remaining = item->getCount();
    not_null_ptr<ItemInstance> removed = ItemInstance::EMPTY;
    while (remaining > 0)
    {
        int slot = findSlotMatchingItem(item, true);
        if (slot < 0)
        {
            break;
        }
        removed = removeItem(slot, remaining);
        remaining -= removed->getCount();
    }
}

int Inventory::removeResourcesWithTags(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        int slot = findSlotMatchingItem(item, false);
        if (slot >= 0)
        {
            removeItem(slot, item->getCount());
            return 1;
        }
    }
    return 0;
}

not_null_ptr<ItemInstance> Inventory::getResourceItem(Item* item)
{
    int slot = getSlot(item, -1, NULL);
    if (slot < 0)
    {
        return ItemInstance::EMPTY;
    }
    return getItem(slot);
}

not_null_ptr<ItemInstance> Inventory::getResourceItem(Item* item, int auxValue)
{
    int slot = getSlot(item, auxValue, NULL);
    if (slot < 0)
    {
        return ItemInstance::EMPTY;
    }
    return getItem(slot);
}

not_null_ptr<ItemInstance> Inventory::getResourceItem(Item* item, int auxValue, CompoundTag* tag)
{
    int slot = getSlot(item, auxValue, tag);
    if (slot < 0)
    {
        return ItemInstance::EMPTY;
    }
    return getItem(slot);
}

bool Inventory::hasResource(Item* item)
{
    return getSlot(item, -1, NULL) >= 0;
}
