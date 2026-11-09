#include "game/network/ClientboundTabListPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundTabListPacket::ClientboundTabListPacket() : header(), footer()
{
}

ClientboundTabListPacket::~ClientboundTabListPacket()
{
}

int ClientboundTabListPacket::getPacketId()
{
    return 220;
}

void ClientboundTabListPacket::read(DataInputStream* input)
{
    header = Packet::readUtf(input, 32767);
    footer = Packet::readUtf(input, 32767);
}

void ClientboundTabListPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(header, output);
    Packet::writeUtf(footer, output);
}

void ClientboundTabListPacket::handle(PacketListener* listener)
{
}
