#include "game/network/ClientboundRotateHeadPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundRotateHeadPacket::ClientboundRotateHeadPacket()
{
}

ClientboundRotateHeadPacket::ClientboundRotateHeadPacket(boost::shared_ptr<Entity> entity, unsigned char yHeadRot)
{
    entityId = entity->getId();
    this->yHeadRot = yHeadRot;
}

int ClientboundRotateHeadPacket::getPacketId()
{
    return 35;
}

void ClientboundRotateHeadPacket::read(DataInputStream* input)
{
    entityId = input->readVarInt();
    yHeadRot = input->readByte();
}

void ClientboundRotateHeadPacket::write(DataOutputStream* output)
{
    output->writeVarInt(entityId);
    output->writeByte(yHeadRot);
}

void ClientboundRotateHeadPacket::handle(PacketListener* listener)
{
    listener->handleRotateMob(shared_from_this());
}

int ClientboundRotateHeadPacket::getEstimatedSize()
{
    return 5;
}

bool ClientboundRotateHeadPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundRotateHeadPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return boost::dynamic_pointer_cast<ClientboundRotateHeadPacket>(packet)->entityId == entityId;
}

bool ClientboundRotateHeadPacket::isAync()
{
    return true;
}

int ClientboundRotateHeadPacket::getEntityId()
{
    return entityId;
}

unsigned char ClientboundRotateHeadPacket::getYHeadRot()
{
    return yHeadRot;
}
