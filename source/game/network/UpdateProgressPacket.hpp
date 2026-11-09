#ifndef GAME_NETWORK_UPDATEPROGRESSPACKET_HPP
#define GAME_NETWORK_UPDATEPROGRESSPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ClientPacketListener;

class UpdateProgressPacket : public Packet, public boost::enable_shared_from_this<UpdateProgressPacket>
{
public:
    UpdateProgressPacket();
    UpdateProgressPacket(int progress);

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new UpdateProgressPacket()); }

    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();

private:
    friend class ClientPacketListener;
    int mProgress;
};

#endif
