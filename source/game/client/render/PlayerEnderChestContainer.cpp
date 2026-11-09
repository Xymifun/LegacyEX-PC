#include "game/client/render/PlayerEnderChestContainer.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"

PlayerEnderChestContainer::PlayerEnderChestContainer() : SimpleContainer(L"", 27)
{
    m_activeChest = boost::shared_ptr<EnderChestBlockEntity>();
}

PlayerEnderChestContainer::~PlayerEnderChestContainer()
{
}

bool PlayerEnderChestContainer::stillValid(boost::shared_ptr<Player> player)
{
    if (m_activeChest && !m_activeChest->stillValid(player))
    {
        return false;
    }
    return SimpleContainer::stillValid(player);
}

void PlayerEnderChestContainer::startOpen(boost::shared_ptr<Player> player)
{
    if (m_activeChest)
    {
        m_activeChest->startOpen();
    }
    SimpleContainer::startOpen(player);
}

void PlayerEnderChestContainer::stopOpen(boost::shared_ptr<Player> player)
{
    if (m_activeChest)
    {
        m_activeChest->stopOpen();
    }
    SimpleContainer::stopOpen(player);
    m_activeChest = boost::shared_ptr<EnderChestBlockEntity>();
}

unsigned int PlayerEnderChestContainer::getContainerType()
{
    return 15;
}

void PlayerEnderChestContainer::setActiveChest(boost::shared_ptr<EnderChestBlockEntity> chest)
{
    m_activeChest = chest;
}

void PlayerEnderChestContainer::setItemsByTag(ListTag* tag)
{
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        setItem(slot, ItemInstance::EMPTY);
    }

    for (int index = 0; index < tag->size(); ++index)
    {
        CompoundTag* itemTag = tag->getCompound(index);
        int slot = itemTag->getByte(L"Slot");
        if (slot >= 0 && slot < getContainerSize())
        {
            setItem(slot, not_null_ptr<ItemInstance>(new ItemInstance(itemTag)));
        }
    }
}

ListTag* PlayerEnderChestContainer::createTag()
{
    ListTag* tag = new ListTag();
    for (unsigned int slot = 0; slot < getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = getItem(slot);
        if (!item->isEmpty())
        {
            CompoundTag* itemTag = new CompoundTag();
            itemTag->putByte(L"Slot", (unsigned char)slot);
            item->save(itemTag);
            tag->add(itemTag);
        }
    }
    return tag;
}

InteractionObject::NameData PlayerEnderChestContainer::GetNetworkNameData()
{
    return m_activeChest ? m_activeChest->GetNetworkNameData()
                         : InteractionObject::NameData(getDisplayName(), L"", true, false);
}
