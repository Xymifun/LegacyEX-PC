#include "game/network/ClientboundSetEquippedItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetEquippedItemPacket::ClientboundSetEquippedItemPacket() : m_item(), m_entity(0), m_slot(0)
{
    m_item = ItemInstance::EMPTY;
}

ClientboundSetEquippedItemPacket::ClientboundSetEquippedItemPacket(int entity, const EquipmentSlot* slot,
                                                                   not_null_ptr<ItemInstance> item)
    : m_item(),
      m_entity(entity),
      m_slot(slot)
{
    m_item = item->isEmpty() ? ItemInstance::EMPTY : item->copy();
}

void ClientboundSetEquippedItemPacket::read(DataInputStream* input)
{
    m_entity = input->readVarInt();
    short slot = input->readShort();
    m_slot = EquipmentSlot::SLOTS[slot];
    m_item = input->readItem();
}

void ClientboundSetEquippedItemPacket::write(DataOutputStream* output)
{
    output->writeVarInt(m_entity);
    output->writeShort(static_cast<short>(m_slot->getEIndex()));
    output->writeItem(m_item);
}

void ClientboundSetEquippedItemPacket::handle(PacketListener* listener)
{
    listener->handleSetEquippedItem(shared_from_this());
}

int ClientboundSetEquippedItemPacket::getEstimatedSize()
{
    return 8;
}

bool ClientboundSetEquippedItemPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetEquippedItemPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundSetEquippedItemPacket> other =
        boost::dynamic_pointer_cast<ClientboundSetEquippedItemPacket>(packet);
    return other->m_entity == m_entity && other->m_slot == m_slot;
}

not_null_ptr<ItemInstance> ClientboundSetEquippedItemPacket::getItem()
{
    return m_item;
}

int ClientboundSetEquippedItemPacket::getEntity()
{
    return m_entity;
}

const EquipmentSlot* ClientboundSetEquippedItemPacket::getSlot()
{
    return m_slot;
}

int ClientboundSetEquippedItemPacket::getPacketId()
{
    return 5;
}
