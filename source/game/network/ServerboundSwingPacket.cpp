#include "game/network/ServerboundSwingPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundSwingPacket::ServerboundSwingPacket()
{
    hand = InteractionHand::eInteractionHand_MainHand;
}

ServerboundSwingPacket::ServerboundSwingPacket(InteractionHand::EInteractionHand hand)
{
    this->hand = hand;
}

void ServerboundSwingPacket::read(DataInputStream* input)
{
    hand = (InteractionHand::EInteractionHand)input->read();
}

void ServerboundSwingPacket::write(DataOutputStream* output)
{
    output->write((unsigned char)hand);
}

void ServerboundSwingPacket::handle(PacketListener* listener)
{
    listener->handleAnimate(shared_from_this());
}

InteractionHand::EInteractionHand ServerboundSwingPacket::getHand()
{
    return hand;
}

int ServerboundSwingPacket::getPacketId()
{
    return 233;
}
