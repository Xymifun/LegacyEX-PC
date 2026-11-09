#include "game/network/ServerboundContainerButtonClickPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"


ServerboundContainerButtonClickPacket::ServerboundContainerButtonClickPacket()
{
}

ServerboundContainerButtonClickPacket::ServerboundContainerButtonClickPacket(int containerId, int buttonId)
{
    this->containerId = containerId;
    this->buttonId = buttonId;
}

int ServerboundContainerButtonClickPacket::getPacketId()
{
    return 108;
}

void ServerboundContainerButtonClickPacket::read(DataInputStream* input)
{
    containerId = input->readByte();
    buttonId = input->readByte();
}

void ServerboundContainerButtonClickPacket::write(DataOutputStream* output)
{
    output->writeByte(containerId);
    output->writeByte(buttonId);
}

void ServerboundContainerButtonClickPacket::handle(PacketListener* listener)
{
    listener->handleContainerButtonClick(shared_from_this());
}

int ServerboundContainerButtonClickPacket::getEstimatedSize()
{
    return 2;
}

int ServerboundContainerButtonClickPacket::getContainerId()
{
    return containerId;
}

int ServerboundContainerButtonClickPacket::getButtonId()
{
    return buttonId;
}
