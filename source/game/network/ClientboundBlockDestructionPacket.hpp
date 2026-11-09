#ifndef GAME_NETWORK_CLIENTBOUNDBLOCKDESTRUCTIONPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDBLOCKDESTRUCTIONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundBlockDestructionPacket : public Packet,
                                          public boost::enable_shared_from_this<ClientboundBlockDestructionPacket>
{
public:
    ClientboundBlockDestructionPacket();
    ClientboundBlockDestructionPacket(int id, const BlockPos& pos, int progress);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getId();
    BlockPos getPos();
    int getProgress();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundBlockDestructionPacket());
    }

private:
    int id;
    BlockPos pos;
    int progress;
};

#endif
