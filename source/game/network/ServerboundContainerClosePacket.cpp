#include "game/network/ServerboundContainerClosePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundContainerClosePacket::ServerboundContainerClosePacket()
{
}

ServerboundContainerClosePacket::ServerboundContainerClosePacket(int containerId) : mContainerId(containerId)
{
}

ServerboundContainerClosePacket::~ServerboundContainerClosePacket()
{
}

void ServerboundContainerClosePacket::read(DataInputStream* input)
{
    mContainerId = input->readByte();
}

void ServerboundContainerClosePacket::write(DataOutputStream* output)
{
    output->writeByte(static_cast<unsigned char>(mContainerId));
}

void ServerboundContainerClosePacket::handle(PacketListener* listener)
{
    listener->handleContainerClose(shared_from_this());
}

int ServerboundContainerClosePacket::getPacketId()
{
    return 224;
}

int ServerboundContainerClosePacket::getContainerId()
{
    return mContainerId;
}
