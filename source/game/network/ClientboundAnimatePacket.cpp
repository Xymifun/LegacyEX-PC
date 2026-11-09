#include "game/network/ClientboundAnimatePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundAnimatePacket::ClientboundAnimatePacket()
{
    action = 0;
    id = -1;
}

ClientboundAnimatePacket::ClientboundAnimatePacket(boost::shared_ptr<Entity> entity, int action)
{
    id = entity->getId();
    this->action = action;
}

int ClientboundAnimatePacket::getPacketId()
{
    return 18;
}

void ClientboundAnimatePacket::read(DataInputStream* input)
{
    id = input->readInt();
    action = input->readByte();
}

void ClientboundAnimatePacket::write(DataOutputStream* output)
{
    output->writeInt(id);
    output->writeByte(action);
}

void ClientboundAnimatePacket::handle(PacketListener* listener)
{
    listener->handleAnimate(shared_from_this());
}

int ClientboundAnimatePacket::getEstimatedSize()
{
    return 5;
}

int ClientboundAnimatePacket::getId()
{
    return id;
}

int ClientboundAnimatePacket::getAction()
{
    return action;
}
