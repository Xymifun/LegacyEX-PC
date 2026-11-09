#include "game/network/ClientboundResourcePackPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundResourcePackPacket::ClientboundResourcePackPacket()
{
    url = L"";
    hash = L"";
}

ClientboundResourcePackPacket::ClientboundResourcePackPacket(const std::wstring& url, const std::wstring& hash)
{
    this->url = url;
    this->hash = hash;
}

ClientboundResourcePackPacket::~ClientboundResourcePackPacket()
{
}

int ClientboundResourcePackPacket::getPacketId()
{
    return 216;
}

void ClientboundResourcePackPacket::read(DataInputStream* input)
{
    url = Packet::readUtf(input, 32767);
    hash = Packet::readUtf(input, 40);
}

void ClientboundResourcePackPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(url, output);
    Packet::writeUtf(hash, output);
}

void ClientboundResourcePackPacket::handle(PacketListener* listener)
{
    listener->handleResourcePack(shared_from_this());
}

std::wstring ClientboundResourcePackPacket::getUrl()
{
    return url;
}

std::wstring ClientboundResourcePackPacket::getHash()
{
    return hash;
}
