#ifndef GAME_NETWORK_CLIENTINTENTIONPACKET_HPP
#define GAME_NETWORK_CLIENTINTENTIONPACKET_HPP

#include <string>

#include "game/network/Packet.hpp"

class ConnectionProtocol;
class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientIntentionPacket : public Packet
{
public:
    ClientIntentionPacket();
    ClientIntentionPacket(int protocolVersion, std::wstring hostName, int port, ConnectionProtocol* intention);

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    ConnectionProtocol* getIntention();
    int getProtocolVersion();
    std::wstring getHostName();
    int getPort();

private:
    int protocolVersion;
    std::wstring hostName;
    int port;
    ConnectionProtocol* intention;
};

#endif
