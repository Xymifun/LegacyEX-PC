#ifndef GAME_NETWORK_CLIENTBOUNDCHUNKBLOCKSUPDATEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDCHUNKBLOCKSUPDATEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class Level;
class PacketListener;

class ClientboundChunkBlocksUpdatePacket : public Packet,
                                           public boost::enable_shared_from_this<ClientboundChunkBlocksUpdatePacket>
{
public:
    ClientboundChunkBlocksUpdatePacket();
    ClientboundChunkBlocksUpdatePacket(int x, int z, arrayWithLength<short> changes, unsigned char count, Level* level);
    virtual ~ClientboundChunkBlocksUpdatePacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundChunkBlocksUpdatePacket());
    }

    friend class ClientPacketListener;

private:
    static int PositionSortFunction(const void* first, const void* second);

    int x;
    int z;
    arrayWithLength<short> positions;
    arrayWithLength<unsigned int> elements;
    unsigned int count;
    int dimension;
};

#endif
