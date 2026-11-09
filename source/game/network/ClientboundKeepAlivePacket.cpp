#include "game/network/ClientboundKeepAlivePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundKeepAlivePacket::ClientboundKeepAlivePacket() : mId(0)
{
}

int ClientboundKeepAlivePacket::getPacketId()
{
    return 0;
}

void ClientboundKeepAlivePacket::read(DataInputStream* input)
{
    mId = input->readInt();
}

void ClientboundKeepAlivePacket::write(DataOutputStream* output)
{
    output->writeInt(mId);
}

void ClientboundKeepAlivePacket::handle(PacketListener* listener)
{
    listener->handleKeepAlive(shared_from_this());
}

int ClientboundKeepAlivePacket::getEstimatedSize()
{
    return 4;
}

bool ClientboundKeepAlivePacket::canBeInvalidated()
{
    return true;
}

bool ClientboundKeepAlivePacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return true;
}

bool ClientboundKeepAlivePacket::isAync()
{
    return true;
}
