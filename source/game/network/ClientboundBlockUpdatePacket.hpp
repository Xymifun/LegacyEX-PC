#ifndef GAME_NETWORK_CLIENTBOUNDBLOCKUPDATEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDBLOCKUPDATEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class BlockState;
class DataInputStream;
class DataOutputStream;
class Level;
class PacketListener;

class ClientboundBlockUpdatePacket : public Packet, public boost::enable_shared_from_this<ClientboundBlockUpdatePacket>
{
public:
    ClientboundBlockUpdatePacket();
    ClientboundBlockUpdatePacket(Level* level, const BlockPos& pos);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    BlockPos getPos();
    const BlockState* getBlock();
    const BlockState* getExtraBlock();
    unsigned int getLevelIdx();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundBlockUpdatePacket()); }

private:
    BlockPos pos;
    const BlockState* block;
    const BlockState* extraBlock;
    unsigned int levelIdx;
};

#endif
