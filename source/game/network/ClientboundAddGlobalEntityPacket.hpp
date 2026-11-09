#ifndef GAME_NETWORK_CLIENTBOUNDADDGLOBALENTITYPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDADDGLOBALENTITYPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
class Entity;

class ClientboundAddGlobalEntityPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundAddGlobalEntityPacket>
{
public:
    ClientboundAddGlobalEntityPacket();
    ClientboundAddGlobalEntityPacket(boost::shared_ptr<Entity> entity);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();

    int getId();
    int getX();
    int getY();
    int getZ();
    int getType();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundAddGlobalEntityPacket());
    }

private:
    int mId;
    int mX;
    int mY;
    int mZ;
    int mType;
};

#endif
