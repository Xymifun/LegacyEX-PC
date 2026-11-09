#include "game/network/ServerboundUseItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundUseItemPacket::ServerboundUseItemPacket()
{
    hand = InteractionHand::eInteractionHand_MainHand;
}

ServerboundUseItemPacket::ServerboundUseItemPacket(InteractionHand::EInteractionHand hand)
{
    this->hand = hand;
}

int ServerboundUseItemPacket::getPacketId()
{
    return 15;
}

void ServerboundUseItemPacket::read(DataInputStream* input)
{
    hand = (InteractionHand::EInteractionHand)input->readByte();
}

void ServerboundUseItemPacket::write(DataOutputStream* output)
{
    output->writeByte((unsigned char)hand);
}

void ServerboundUseItemPacket::handle(PacketListener* listener)
{
    listener->handleUseItem(shared_from_this());
}

int ServerboundUseItemPacket::getEstimatedSize()
{
    return 1;
}

InteractionHand::EInteractionHand ServerboundUseItemPacket::getHand()
{
    return hand;
}
