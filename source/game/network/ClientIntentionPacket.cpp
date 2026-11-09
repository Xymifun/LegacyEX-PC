#include "game/network/ClientIntentionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientIntentionPacket::ClientIntentionPacket() : Packet(), protocolVersion(0), hostName(), port(0), intention(NULL)
{
}

ClientIntentionPacket::ClientIntentionPacket(int protocolVersionValue, std::wstring hostNameValue, int portValue,
                                             ConnectionProtocol* intentionValue)
    : Packet(),
      protocolVersion(protocolVersionValue),
      hostName(hostNameValue),
      port(portValue),
      intention(intentionValue)
{
}

void ClientIntentionPacket::read(DataInputStream* input)
{
}

void ClientIntentionPacket::write(DataOutputStream* output)
{
}

void ClientIntentionPacket::handle(PacketListener* listener)
{
}

ConnectionProtocol* ClientIntentionPacket::getIntention()
{
    return intention;
}

int ClientIntentionPacket::getProtocolVersion()
{
    return protocolVersion;
}

std::wstring ClientIntentionPacket::getHostName()
{
    return hostName;
}

int ClientIntentionPacket::getPort()
{
    return port;
}
