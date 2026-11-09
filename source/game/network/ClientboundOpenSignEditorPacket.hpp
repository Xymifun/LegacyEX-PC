#ifndef GAME_NETWORK_CLIENTBOUNDOPENSIGNEDITORPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDOPENSIGNEDITORPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundOpenSignEditorPacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundOpenSignEditorPacket>
{
public:
    ClientboundOpenSignEditorPacket();
    ClientboundOpenSignEditorPacket(const BlockPos& pos);
    virtual ~ClientboundOpenSignEditorPacket();
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    BlockPos getPos();
    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundOpenSignEditorPacket());
    }

private:
    BlockPos pos;
};

#endif
