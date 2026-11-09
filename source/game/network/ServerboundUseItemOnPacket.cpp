#include "game/network/ServerboundUseItemOnPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/network/PacketListener.hpp"

ServerboundUseItemOnPacket::ServerboundUseItemOnPacket()
{
}

ServerboundUseItemOnPacket::ServerboundUseItemOnPacket(const BlockPos& pos, const Direction* face,
                                                       InteractionHand::EInteractionHand hand, float clickX,
                                                       float clickY, float clickZ)
{
    this->pos = pos;
    this->face = face;
    this->hand = hand;
    this->clickX = clickX;
    this->clickY = clickY;
    this->clickZ = clickZ;
}

int ServerboundUseItemOnPacket::getPacketId()
{
    return 137;
}

void ServerboundUseItemOnPacket::read(DataInputStream* input)
{
    pos = input->readBlockPos();
    face = Direction::from3DDataValue(input->readByte());
    hand = static_cast<InteractionHand::EInteractionHand>(input->readByte());
    clickX = input->readFloat();
    clickY = input->readFloat();
    clickZ = input->readFloat();
}

void ServerboundUseItemOnPacket::write(DataOutputStream* output)
{
    output->writeBlockPos(pos);
    output->writeByte(static_cast<unsigned char>(face->get3DDataValue()));
    output->writeByte(static_cast<unsigned char>(hand));
    output->writeFloat(clickX);
    output->writeFloat(clickY);
    output->writeFloat(clickZ);
}

void ServerboundUseItemOnPacket::handle(PacketListener* listener)
{
    listener->handleUseItemOn(shared_from_this());
}

BlockPos ServerboundUseItemOnPacket::getPos()
{
    return pos;
}

const Direction* ServerboundUseItemOnPacket::getFace()
{
    return face;
}

InteractionHand::EInteractionHand ServerboundUseItemOnPacket::getHand()
{
    return hand;
}

double ServerboundUseItemOnPacket::getClickX()
{
    return clickX;
}

double ServerboundUseItemOnPacket::getClickY()
{
    return clickY;
}

double ServerboundUseItemOnPacket::getClickZ()
{
    return clickZ;
}
