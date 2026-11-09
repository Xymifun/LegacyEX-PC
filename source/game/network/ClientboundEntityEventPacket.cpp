#include "game/network/ClientboundEntityEventPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundEntityEventPacket::ClientboundEntityEventPacket()
{
    entityId = 0;
    eventId = 0;
    data = 0;
}

ClientboundEntityEventPacket::ClientboundEntityEventPacket(boost::shared_ptr<Entity> entity, unsigned char eventId,
                                                           int data)
{
    entityId = entity->getId();
    this->eventId = eventId;
    this->data = data;
}

int ClientboundEntityEventPacket::getPacketId()
{
    return 38;
}

void ClientboundEntityEventPacket::read(DataInputStream* input)
{
    entityId = input->readInt();
    eventId = input->readByte();
    data = input->readInt();
}

void ClientboundEntityEventPacket::write(DataOutputStream* output)
{
    output->writeInt(entityId);
    output->writeByte(eventId);
    output->writeInt(data);
}

void ClientboundEntityEventPacket::handle(PacketListener* listener)
{
    listener->handleEntityEvent(shared_from_this());
}

int ClientboundEntityEventPacket::getEstimatedSize()
{
    return 9;
}

int ClientboundEntityEventPacket::getEntityId() const
{
    return entityId;
}

unsigned char ClientboundEntityEventPacket::getEventId() const
{
    return eventId;
}

int ClientboundEntityEventPacket::getData() const
{
    return data;
}
