#include "game/network/ClientboundDamageIndicatorPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundDamageIndicatorPacket::ClientboundDamageIndicatorPacket()
    : Packet(),
      boost::enable_shared_from_this<ClientboundDamageIndicatorPacket>(),
      x(0.0f),
      z(0.0f),
      allDirections(true)
{
}

ClientboundDamageIndicatorPacket::ClientboundDamageIndicatorPacket(float xValue, float zValue)
    : Packet(),
      boost::enable_shared_from_this<ClientboundDamageIndicatorPacket>(),
      x(xValue),
      z(zValue),
      allDirections(false)
{
}

void ClientboundDamageIndicatorPacket::read(DataInputStream* input)
{
    x = input->readFloat();
    z = input->readFloat();
    allDirections = input->readBoolean();
}

void ClientboundDamageIndicatorPacket::write(DataOutputStream* output)
{
    output->writeFloat(x);
    output->writeFloat(z);
    output->writeBoolean(allDirections);
}

void ClientboundDamageIndicatorPacket::handle(PacketListener* listener)
{
    listener->handleDamageIndicator(shared_from_this());
}

int ClientboundDamageIndicatorPacket::getEstimatedSize()
{
    return 9;
}

double ClientboundDamageIndicatorPacket::getX()
{
    return (double)x;
}

double ClientboundDamageIndicatorPacket::getZ()
{
    return (double)z;
}

unsigned char ClientboundDamageIndicatorPacket::isAllDirections()
{
    return allDirections;
}

int ClientboundDamageIndicatorPacket::getPacketId()
{
    return 234;
}
