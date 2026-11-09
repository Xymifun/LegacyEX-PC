#ifndef GAME_NETWORK_SERVERBOUNDPRELOGINPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDPRELOGINPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ServerConnectionListener;

class ServerboundPreLoginPacket : public Packet, public boost::enable_shared_from_this<ServerboundPreLoginPacket>
{
    friend class ServerConnectionListener;

public:
    ServerboundPreLoginPacket();
    ServerboundPreLoginPacket(const std::wstring& name);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundPreLoginPacket()); }

private:
    short protocolVersion;
    std::wstring name;
};

#endif
