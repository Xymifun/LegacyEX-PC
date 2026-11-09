#ifndef GAME_NETWORK_CLIENTBOUNDSETENTITYLINKPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETENTITYLINKPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundSetEntityLinkPacket : public Packet,
                                       public boost::enable_shared_from_this<ClientboundSetEntityLinkPacket>
{
public:
    ClientboundSetEntityLinkPacket();
    ClientboundSetEntityLinkPacket(boost::shared_ptr<Entity> source, boost::shared_ptr<Entity> dest);
    ClientboundSetEntityLinkPacket(int type, boost::shared_ptr<Entity> source, boost::shared_ptr<Entity> dest);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);

    int getSourceId();
    int getDestId();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetEntityLinkPacket());
    }

private:
    int sourceId;
    int destId;
};

#endif
