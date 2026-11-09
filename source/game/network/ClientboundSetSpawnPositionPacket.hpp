#ifndef GAME_NETWORK_CLIENTBOUNDSETSPAWNPOSITIONPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETSPAWNPOSITIONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class ClientPacketListener;
class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundSetSpawnPositionPacket : public Packet,
                                          public boost::enable_shared_from_this<ClientboundSetSpawnPositionPacket>
{
public:
    friend class ClientPacketListener;

    ClientboundSetSpawnPositionPacket();
    ClientboundSetSpawnPositionPacket(int x, int y, int z);
    ClientboundSetSpawnPositionPacket(const BlockPos& pos);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual bool isAync();
    virtual int getPacketId();
    BlockPos getPos() { return mPos; }
    static boost::shared_ptr<Packet> create();

private:
    BlockPos mPos;
};

#endif
