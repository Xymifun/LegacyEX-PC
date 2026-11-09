#ifndef GAME_NETWORK_CHUNKVISIBILITYPACKET_HPP
#define GAME_NETWORK_CHUNKVISIBILITYPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ClientPacketListener;

class ChunkVisibilityPacket : public Packet, public boost::enable_shared_from_this<ChunkVisibilityPacket>
{
public:
    ChunkVisibilityPacket();
    ChunkVisibilityPacket(int x, int z, bool visible, bool loaded);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ChunkVisibilityPacket()); }

private:
    friend class ClientPacketListener;
    int x;
    int z;
    bool visible;
    bool loaded;
};

#endif
