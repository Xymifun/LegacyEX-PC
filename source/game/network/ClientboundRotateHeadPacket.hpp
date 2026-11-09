#ifndef GAME_NETWORK_CLIENTBOUNDROTATEHEADPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDROTATEHEADPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundRotateHeadPacket : public Packet, public boost::enable_shared_from_this<ClientboundRotateHeadPacket>
{
public:
    ClientboundRotateHeadPacket();
    ClientboundRotateHeadPacket(boost::shared_ptr<Entity> entity, unsigned char yHeadRot);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual bool isAync();

    int getEntityId();
    unsigned char getYHeadRot();

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundRotateHeadPacket()); }

private:
    int entityId;
    unsigned char yHeadRot;
};

#endif
