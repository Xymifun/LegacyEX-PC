#ifndef GAME_NETWORK_CLIENTBOUNDMOVEENTITYPACKET_HPP
#define GAME_NETWORK_CLIENTBOUNDMOVEENTITYPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ClientboundMoveEntityPacket : public Packet, public boost::enable_shared_from_this<ClientboundMoveEntityPacket>
{
public:
    ClientboundMoveEntityPacket();
    explicit ClientboundMoveEntityPacket(int entityId);
    virtual ~ClientboundMoveEntityPacket() {}
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    int getEntityId();
    int getXa();
    int getYa();
    int getZa();
    unsigned char getyRot();
    unsigned char getxRot();
    bool hasRotation();
    bool isOnGround();
    static boost::shared_ptr<Packet> create();
    class Pos;
    class Rot;
    class PosRot;

protected:
    int mEntityId;
    int mXa;
    int mYa;
    int mZa;
    unsigned char mYRot;
    unsigned char mXRot;
    bool mOnGround;
    bool mHasRotation;
};

class ClientboundMoveEntityPacket::Pos : public ClientboundMoveEntityPacket
{
public:
    Pos();
    Pos(int entityId, int xa, int ya, int za, bool onGround);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create();
};

class ClientboundMoveEntityPacket::Rot : public ClientboundMoveEntityPacket
{
public:
    Rot();
    Rot(int entityId, unsigned char yRot, unsigned char xRot, bool onGround);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create();
};

class ClientboundMoveEntityPacket::PosRot : public ClientboundMoveEntityPacket
{
public:
    PosRot();
    PosRot(int entityId, int xa, int ya, int za, unsigned char yRot, unsigned char xRot, bool onGround);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create();
};

#endif
