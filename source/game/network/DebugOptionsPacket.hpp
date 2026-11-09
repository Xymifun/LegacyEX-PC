#ifndef GAME_NETWORK_DEBUGOPTIONSPACKET_HPP
#define GAME_NETWORK_DEBUGOPTIONSPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ServerGamePacketListenerImpl;

class DebugOptionsPacket : public Packet, public boost::enable_shared_from_this<DebugOptionsPacket>
{
public:
    DebugOptionsPacket();
    DebugOptionsPacket(unsigned int options);
    virtual ~DebugOptionsPacket();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new DebugOptionsPacket()); }

private:
    friend class ServerGamePacketListenerImpl;

    unsigned int options;
};

#endif
