#ifndef GAME_NETWORK_CLIENTBOUNDENTITYEVENTPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDENTITYEVENTPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundEntityEventPacket : public Packet, public boost::enable_shared_from_this<ClientboundEntityEventPacket>
{
public:
    ClientboundEntityEventPacket();
    ClientboundEntityEventPacket(boost::shared_ptr<Entity> entity, unsigned char eventId, int data);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getEntityId() const;
    unsigned char getEventId() const;
    int getData() const;

    static boost::shared_ptr<Packet> create() { return boost::shared_ptr<Packet>(new ClientboundEntityEventPacket()); }

private:
    int entityId;
    unsigned char eventId;
    int data;
};

#endif
