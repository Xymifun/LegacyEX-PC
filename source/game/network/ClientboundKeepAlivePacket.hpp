#ifndef GAME_NETWORK_CLIENTBOUNDKEEPALIVEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDKEEPALIVEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundKeepAlivePacket : public Packet, public boost::enable_shared_from_this<ClientboundKeepAlivePacket>
{
public:
    ClientboundKeepAlivePacket();
    ClientboundKeepAlivePacket(int id);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual bool isAync();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundKeepAlivePacket()); }

private:
    int mId;
};

#endif
