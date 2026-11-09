#ifndef GAME_NETWORK_CLIENTBOUNDBLOCKENTITYDATAPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDBLOCKENTITYDATAPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundBlockEntityDataPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundBlockEntityDataPacket>
{
public:
    ClientboundBlockEntityDataPacket();
    ClientboundBlockEntityDataPacket(const BlockPos& pos, int type, int dimension, CompoundTag* tag);
    virtual ~ClientboundBlockEntityDataPacket();

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    BlockPos getPos();
    int getType();
    CompoundTag* getTag();
    int getDimension() const;

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundBlockEntityDataPacket());
    }

private:
    void _init();

    BlockPos pos;
    int type;
    CompoundTag* tag;
    int dimension;
};

#endif
