#include "game/network/ServerboundPreLoginPacket.hpp"
#include "game/core/SharedConstants.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundPreLoginPacket::ServerboundPreLoginPacket()
{
    name = L"";
    protocolVersion = 0;
}

ServerboundPreLoginPacket::ServerboundPreLoginPacket(const std::wstring& name) : name(name)
{
    protocolVersion = 0;
}

int ServerboundPreLoginPacket::getPacketId()
{
    return 232;
}

void ServerboundPreLoginPacket::read(DataInputStream* input)
{
    protocolVersion = input->readShort();
    name = Packet::readUtf(input, 32);
}

void ServerboundPreLoginPacket::write(DataOutputStream* output)
{
    output->writeShort(SharedConstants::NETWORK_PROTOCOL_VERSION);
    Packet::writeUtf(name, output);
}

void ServerboundPreLoginPacket::handle(PacketListener* listener)
{
    listener->handlePreLogin(shared_from_this());
}

int ServerboundPreLoginPacket::getEstimatedSize()
{
    return name.length() * 2 + 2;
}
