#ifndef GAME_NETWORK_CHUNKVISIBILITYAREAPACKET_HPP
#define GAME_NETWORK_CHUNKVISIBILITYAREAPACKET_HPP

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ChunkVisibilityAreaPacket : public Packet, public boost::enable_shared_from_this<ChunkVisibilityAreaPacket>
{
public:
    ChunkVisibilityAreaPacket();
    ChunkVisibilityAreaPacket(int minX, int maxX, int minZ, int maxZ, bool visible, bool loaded, bool outside);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ChunkVisibilityAreaPacket()); }

    friend class ClientPacketListener;

private:
    int minX;
    int maxX;
    int minZ;
    int maxZ;
    bool visible;
    bool loaded;
    bool outside;
};

#endif
