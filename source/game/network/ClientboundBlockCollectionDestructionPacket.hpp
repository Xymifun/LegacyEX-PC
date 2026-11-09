#ifndef GAME_NETWORK_CLIENTBOUNDBLOCKCOLLECTIONDESTRUCTIONPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDBLOCKCOLLECTIONDESTRUCTIONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class ClientPacketListener;

class ClientboundBlockCollectionDestructionPacket
    : public Packet,
      public boost::enable_shared_from_this<ClientboundBlockCollectionDestructionPacket>
{
public:
    ClientboundBlockCollectionDestructionPacket();
    ClientboundBlockCollectionDestructionPacket(int sourceId, arrayWithLength<BlockPos> positions, int progress);
    virtual ~ClientboundBlockCollectionDestructionPacket();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundBlockCollectionDestructionPacket());
    }

private:
    friend class ClientPacketListener;
    int sourceId;
    int progress;
    arrayWithLength<BlockPos> positions;
};

#endif
