#include "game/network/ServerboundPaddleBoatPacket.hpp"


#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundPaddleBoatPacket::ServerboundPaddleBoatPacket()
{
}

ServerboundPaddleBoatPacket::ServerboundPaddleBoatPacket(bool left, bool right)
{
    this->left = left;
    this->right = right;
}

int ServerboundPaddleBoatPacket::getPacketId()
{
    return 138;
}

void ServerboundPaddleBoatPacket::read(DataInputStream* input)
{
    left = input->readBoolean();
    right = input->readBoolean();
}

void ServerboundPaddleBoatPacket::write(DataOutputStream* output)
{
    output->writeBoolean(left);
    output->writeBoolean(right);
}

void ServerboundPaddleBoatPacket::handle(PacketListener* listener)
{
    listener->handlePaddleBoat(shared_from_this());
}

bool ServerboundPaddleBoatPacket::getLeft()
{
    return left;
}

bool ServerboundPaddleBoatPacket::getRight()
{
    return right;
}
