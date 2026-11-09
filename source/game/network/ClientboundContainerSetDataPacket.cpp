#include "game/network/ClientboundContainerSetDataPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundContainerSetDataPacket::ClientboundContainerSetDataPacket()
{
    containerId = 0;
    id = -1;
    value = 0;
}

ClientboundContainerSetDataPacket::ClientboundContainerSetDataPacket(int containerId, int id, int value)
{
    this->containerId = containerId;
    this->id = id;
    this->value = value;
}

int ClientboundContainerSetDataPacket::getPacketId()
{
    return 105;
}

void ClientboundContainerSetDataPacket::read(DataInputStream* input)
{
    containerId = input->readUnsignedByte();
    id = input->readShort();
    value = input->readShort();
}

void ClientboundContainerSetDataPacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
    output->writeShort(id);
    output->writeShort(value);
}

void ClientboundContainerSetDataPacket::handle(PacketListener* listener)
{
    listener->handleContainerSetData(shared_from_this());
}

int ClientboundContainerSetDataPacket::getEstimatedSize()
{
    return 5;
}

int ClientboundContainerSetDataPacket::getContainerId()
{
    return containerId;
}

int ClientboundContainerSetDataPacket::getId()
{
    return id;
}

int ClientboundContainerSetDataPacket::getValue()
{
    return value;
}
