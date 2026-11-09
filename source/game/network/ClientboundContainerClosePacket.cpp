#include "game/network/ClientboundContainerClosePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundContainerClosePacket::ClientboundContainerClosePacket() : containerId(0)
{
}

ClientboundContainerClosePacket::ClientboundContainerClosePacket(int containerId) : containerId(containerId)
{
}

int ClientboundContainerClosePacket::getPacketId()
{
    return 101;
}

void ClientboundContainerClosePacket::read(DataInputStream* input)
{
    containerId = input->readByte();
}

void ClientboundContainerClosePacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
}

void ClientboundContainerClosePacket::handle(PacketListener* listener)
{
    listener->handleContainerClose(shared_from_this());
}

int ClientboundContainerClosePacket::getEstimatedSize()
{
    return 1;
}
