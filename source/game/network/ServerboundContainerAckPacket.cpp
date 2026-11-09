#include "game/network/ServerboundContainerAckPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"


ServerboundContainerAckPacket::ServerboundContainerAckPacket()
{
}

ServerboundContainerAckPacket::ServerboundContainerAckPacket(int containerId, short uid, bool accepted)
{
    this->containerId = containerId;
    this->uid = uid;
    this->accepted = accepted;
}

ServerboundContainerAckPacket::~ServerboundContainerAckPacket()
{
}

int ServerboundContainerAckPacket::getPacketId()
{
    return 223;
}

void ServerboundContainerAckPacket::read(DataInputStream* input)
{
    containerId = input->readByte();
    uid = input->readShort();
    accepted = input->readBoolean();
}

void ServerboundContainerAckPacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
    output->writeShort(uid);
    output->writeBoolean(accepted);
}

void ServerboundContainerAckPacket::handle(PacketListener* listener)
{
    listener->handleContainerAck(shared_from_this());
}

int ServerboundContainerAckPacket::getContainerId()
{
    return containerId;
}

int ServerboundContainerAckPacket::getUid()
{
    return uid;
}

bool ServerboundContainerAckPacket::isAccepted()
{
    return accepted;
}
