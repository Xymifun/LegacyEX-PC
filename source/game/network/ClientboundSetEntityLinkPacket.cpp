#include "game/network/ClientboundSetEntityLinkPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetEntityLinkPacket::ClientboundSetEntityLinkPacket()
{
    sourceId = -1;
    destId = -1;
}

ClientboundSetEntityLinkPacket::ClientboundSetEntityLinkPacket(boost::shared_ptr<Entity> source,
                                                               boost::shared_ptr<Entity> dest)
{
    sourceId = source->getId();
    destId = dest != NULL ? dest->getId() : -1;
}

ClientboundSetEntityLinkPacket::ClientboundSetEntityLinkPacket(int type, boost::shared_ptr<Entity> source,
                                                               boost::shared_ptr<Entity> dest)
{
    sourceId = source->getId();
    destId = dest != NULL ? dest->getId() : -1;
}

int ClientboundSetEntityLinkPacket::getPacketId()
{
    return 39;
}

void ClientboundSetEntityLinkPacket::read(DataInputStream* input)
{
    sourceId = input->readInt();
    destId = input->readInt();
}

void ClientboundSetEntityLinkPacket::write(DataOutputStream* output)
{
    output->writeInt(sourceId);
    output->writeInt(destId);
}

void ClientboundSetEntityLinkPacket::handle(PacketListener* listener)
{
    listener->handleEntityLinkPacket(shared_from_this());
}

int ClientboundSetEntityLinkPacket::getEstimatedSize()
{
    return 8;
}

bool ClientboundSetEntityLinkPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetEntityLinkPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundSetEntityLinkPacket> other =
        boost::dynamic_pointer_cast<ClientboundSetEntityLinkPacket>(packet);

    return other->sourceId == sourceId;
}

int ClientboundSetEntityLinkPacket::getSourceId()
{
    return sourceId;
}

int ClientboundSetEntityLinkPacket::getDestId()
{
    return destId;
}
