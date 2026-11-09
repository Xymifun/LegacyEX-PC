#include "game/network/GetInfoPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

int GetInfoPacket::getPacketId()
{
    return 254;
}

void GetInfoPacket::read(DataInputStream* input)
{
}

void GetInfoPacket::write(DataOutputStream* output)
{
}

void GetInfoPacket::handle(PacketListener* listener)
{
    listener->handleGetInfo(shared_from_this());
}

int GetInfoPacket::getEstimatedSize()
{
    return 0;
}
