#ifndef GAME_NETWORK_XZPACKET_HPP
#define GAME_NETWORK_XZPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class XZPacket : public Packet, public boost::enable_shared_from_this<XZPacket>
{
public:
    XZPacket();
    XZPacket(char mode, int x, int z);
    virtual ~XZPacket();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new XZPacket()); }

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    virtual int getPacketId();

    static const int STRONGHOLD;

    char mMode;
    int mX;
    int mZ;
};

#endif
