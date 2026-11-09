#ifndef GAME_NETWORK_CLIENTBOUNDTELEPORTENTITYPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDTELEPORTENTITYPACKET_HPP
#include <boost/enable_shared_from_this.hpp>
#include "game/network/Packet.hpp"

#include <boost/shared_ptr.hpp>
class Entity;
class DataInputStream;
class DataOutputStream;
class PacketListener;
class ClientboundTeleportEntityPacket : public Packet,
                                        public boost::enable_shared_from_this<ClientboundTeleportEntityPacket>
{
public:
    ClientboundTeleportEntityPacket();
    explicit ClientboundTeleportEntityPacket(boost::shared_ptr<Entity> entity);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    int getId();
    int getX();
    int getY();
    int getZ();
    unsigned char getyRot();
    unsigned char getxRot();
    unsigned char isOnGround();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundTeleportEntityPacket());
    }

private:
    int mId;
    int mX;
    int mY;
    int mZ;
    unsigned char myRot;
    unsigned char mxRot;
    bool mOnGround;
};
#endif
