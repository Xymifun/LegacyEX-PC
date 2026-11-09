#ifndef GAME_NETWORK_CLIENTBOUNDSETENTITYMOTIONPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDSETENTITYMOTIONPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class Entity;
class PacketListener;

class ClientboundSetEntityMotionPacket : public Packet,
                                         public boost::enable_shared_from_this<ClientboundSetEntityMotionPacket>
{
public:
    ClientboundSetEntityMotionPacket();
    ClientboundSetEntityMotionPacket(boost::shared_ptr<Entity> entity);
    ClientboundSetEntityMotionPacket(int id, double xa, double ya, double za);

    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);

    int getId();
    int getXa();
    int getYa();
    int getZa();

    static boost::shared_ptr<Packet> create()
    {
        return boost::shared_ptr<Packet>(new ClientboundSetEntityMotionPacket());
    }

private:
    void _init(int id, double xa, double ya, double za);

    bool compact;
    int id;
    int xa;
    int ya;
    int za;
};

#endif
