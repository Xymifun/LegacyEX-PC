#ifndef GAME_NETWORK_CLIENTBOUNDANIMATEPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDANIMATEPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundAnimatePacket : public Packet, public boost::enable_shared_from_this<ClientboundAnimatePacket>
{
public:
    ClientboundAnimatePacket();
    ClientboundAnimatePacket(boost::shared_ptr<Entity> entity, int action);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getId();
    int getAction();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundAnimatePacket()); }

private:
    int id;
    int action;
};

#endif
