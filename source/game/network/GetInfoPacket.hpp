#ifndef GAME_NETWORK_GETINFOPACKET_HPP
#define GAME_NETWORK_GETINFOPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class GetInfoPacket : public Packet, public boost::enable_shared_from_this<GetInfoPacket>
{
public:
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new GetInfoPacket()); }
};

#endif
