#ifndef GAME_NETWORK_SERVERBOUNDMOVEPLAYERPACKET_HPP
#define GAME_NETWORK_SERVERBOUNDMOVEPLAYERPACKET_HPP

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/network/Packet.hpp"

class DataInputStream;
class DataOutputStream;
class PacketListener;

class ServerboundMovePlayerPacket : public Packet, public boost::enable_shared_from_this<ServerboundMovePlayerPacket>
{
public:
    ServerboundMovePlayerPacket();
    ServerboundMovePlayerPacket(bool onGround, bool isFlying);
    virtual ~ServerboundMovePlayerPacket() {}

    virtual void handle(PacketListener* listener);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    virtual bool canBeInvalidated();
    virtual bool isInvalidatedBy(boost::shared_ptr<Packet> packet);
    virtual int getPacketId();

    double getX(double defaultValue);
    double getY(double defaultValue);
    double getZ(double defaultValue);
    float getYRot(float defaultValue);
    float getXRot(float defaultValue);
    bool isOnGround();
    bool hasPosition();
    bool hasRotation();
    void setHasPosition(bool value);
    bool isFlying();
    void setIsFlying(bool value);

    static boost::shared_ptr<Packet> create();

protected:
    double x;
    double y;
    double z;
    float yRot;
    float xRot;
    bool onGround;
    bool hasPositionValue;
    bool hasRotationValue;
    bool flying;

public:
    class Pos;
    typedef class Pos* PosPtr;
    class PosRot;
    typedef class PosRot* PosRotPtr;
    class Rot;
    typedef class Rot* RotPtr;
};

class ServerboundMovePlayerPacket::Pos : public ServerboundMovePlayerPacket
{
public:
    Pos();
    Pos(double x, double y, double unused, double z, bool onGround, bool isFlying);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();
};

class ServerboundMovePlayerPacket::PosRot : public ServerboundMovePlayerPacket
{
public:
    PosRot();
    PosRot(double x, double y, double z, float yRot, float xRot, bool onGround, bool isFlying);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();
};

class ServerboundMovePlayerPacket::Rot : public ServerboundMovePlayerPacket
{
public:
    Rot();
    Rot(float yRot, float xRot, bool onGround, bool isFlying);
    virtual void read(DataInputStream* input);
    virtual void write(DataOutputStream* output);
    virtual int getEstimatedSize();
    virtual int getPacketId();
    static boost::shared_ptr<Packet> create();
};

#endif
