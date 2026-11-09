#ifndef GAME_NETWORK_CLIENTBOUNDLEVELEVENTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDLEVELEVENTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundLevelEventPacket : public Packet, public boost::enable_shared_from_this<ClientboundLevelEventPacket>
{
public:
    ClientboundLevelEventPacket();
    ClientboundLevelEventPacket(int type, const BlockPos& pos, int data, bool globalEvent);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    bool isGlobalEvent();
    int getType();
    int getData();
    BlockPos getPos();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundLevelEventPacket()); }

private:
    int type;
    int data;
    BlockPos pos;
    bool globalEvent;
};

#endif
