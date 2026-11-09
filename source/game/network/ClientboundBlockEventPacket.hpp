#ifndef GAME_NETWORK_CLIENTBOUNDBLOCKEVENTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDBLOCKEVENTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundBlockEventPacket : public Packet, public boost::enable_shared_from_this<ClientboundBlockEventPacket>
{
public:
    ClientboundBlockEventPacket();
    ClientboundBlockEventPacket(const BlockPos& pos, Block* block, int b0, int b1);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    BlockPos getPos();
    int getB0();
    int getB1();
    Block* getBlock();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundBlockEventPacket()); }

private:
    BlockPos pos;
    int b0;
    int b1;
    Block* block;
};

#endif
