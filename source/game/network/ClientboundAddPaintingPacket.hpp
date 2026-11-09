#ifndef GAME_NETWORK_CLIENTBOUNDADDPAINTINGPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDADDPAINTINGPACKET_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"
#include "game/world/block/BlockPos.hpp"

class DataInputStream;
class DataOutputStream;
class Painting;
class PacketListener;

class ClientboundAddPaintingPacket : public Packet, public boost::enable_shared_from_this<ClientboundAddPaintingPacket>
{
public:
    ClientboundAddPaintingPacket();
    ClientboundAddPaintingPacket(boost::shared_ptr<Painting> painting);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getId();
    BlockPos getPos();
    const Direction* getDirection();
    std::wstring getMotive();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundAddPaintingPacket()); }

private:
    int id;
    BlockPos pos;
    const Direction* direction;
    std::wstring motive;
};
#endif
