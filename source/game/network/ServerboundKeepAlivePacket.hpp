#ifndef GAME_NETWORK_SERVERBOUNDKEEPALIVEPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDKEEPALIVEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundKeepAlivePacket : public Packet, public boost::enable_shared_from_this<ServerboundKeepAlivePacket>
{
public:
    ServerboundKeepAlivePacket();
    ServerboundKeepAlivePacket(int id);
    virtual ~ServerboundKeepAlivePacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ServerboundKeepAlivePacket()); }

private:
    friend class ServerGamePacketListenerImpl;
    int id;
};

#endif
