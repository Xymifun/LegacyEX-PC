#ifndef GAME_NETWORK_CLIENTBOUNDTABLISTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDTABLISTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundTabListPacket : public Packet, public boost::enable_shared_from_this<ClientboundTabListPacket>
{
public:
    ClientboundTabListPacket();
    ClientboundTabListPacket(const std::wstring& header);
    virtual ~ClientboundTabListPacket();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundTabListPacket()); }

private:
    std::wstring header;
    std::wstring footer;
};

#endif
