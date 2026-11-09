#ifndef GAME_NETWORK_MOVEENTITYPACKETSMALL_HPP
#define GAME_NETWORK_MOVEENTITYPACKETSMALL_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;
namespace Pos
{
}
namespace PosRot
{
}
namespace Rot
{
}

class MoveEntityPacketSmall : public Packet, public boost::enable_shared_from_this<MoveEntityPacketSmall>
{
    friend class ClientPacketListener;

public:
    MoveEntityPacketSmall();
    explicit MoveEntityPacketSmall(int entityId);
    virtual ~MoveEntityPacketSmall() {}
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual void handle(PacketListener* listener);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual bool isOnGround() const;
    static boost::shared_ptr<Packet> create();
    class Pos;
    class PosRot;
    class Rot;

protected:
    int mEntityId;
    signed char mXa;
    signed char mYa;
    signed char mZa;
    signed char myRot;
    signed char mxRot;
    signed char mHasRot;
    bool mOnGround;
};

class MoveEntityPacketSmall::Pos : public MoveEntityPacketSmall
{
public:
    Pos();
    Pos(int entityId, char xa, char ya, char za, bool onGround);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create();
};
class MoveEntityPacketSmall::PosRot : public MoveEntityPacketSmall
{
public:
    PosRot();
    PosRot(int entityId, char xa, char ya, char za, char yRot, char xRot, bool onGround);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create();
};
class MoveEntityPacketSmall::Rot : public MoveEntityPacketSmall
{
public:
    Rot();
    Rot(int entityId, char yRot, char xRot, bool onGround);
    virtual int getPacketId();
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    static boost::shared_ptr<Packet> create();
};

#endif
