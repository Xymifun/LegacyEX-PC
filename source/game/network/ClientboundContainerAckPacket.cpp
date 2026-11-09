#include "game/network/ClientboundContainerAckPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundContainerAckPacket::ClientboundContainerAckPacket()
{
    containerId = 0;
    uid = 0;
    accepted = false;
}

ClientboundContainerAckPacket::ClientboundContainerAckPacket(int containerId, short uid, bool accepted)
{
    this->containerId = containerId;
    this->uid = uid;
    this->accepted = accepted;
}

int ClientboundContainerAckPacket::getPacketId()
{
    return 106;
}

void ClientboundContainerAckPacket::read(DataInputStream* input)
{
    containerId = input->readByte();
    uid = input->readShort();
    accepted = (input->readByte() != 0);
}

void ClientboundContainerAckPacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
    output->writeShort(uid);
    output->writeByte(accepted);
}

void ClientboundContainerAckPacket::handle(PacketListener* listener)
{
    listener->handleContainerAck(shared_from_this());
}

int ClientboundContainerAckPacket::getEstimatedSize()
{
    return 4;
}

int ClientboundContainerAckPacket::getContainerId()
{
    return containerId;
}

short ClientboundContainerAckPacket::getUid()
{
    return uid;
}

bool ClientboundContainerAckPacket::isAccepted()
{
    return accepted;
}
