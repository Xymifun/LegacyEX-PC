#include "game/network/ServerboundMovePlayerPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundMovePlayerPacket::ServerboundMovePlayerPacket()
    : x(0.0),
      y(0.0),
      z(0.0),
      yRot(0.0f),
      xRot(0.0f),
      onGround(false),
      hasPositionValue(false),
      hasRotationValue(false),
      flying(false)
{
}

ServerboundMovePlayerPacket::ServerboundMovePlayerPacket(bool onGround, bool isFlying)
    : x(0.0),
      y(0.0),
      z(0.0),
      yRot(0.0f),
      xRot(0.0f),
      onGround(onGround),
      hasPositionValue(false),
      hasRotationValue(false),
      flying(isFlying)
{
}

void ServerboundMovePlayerPacket::handle(PacketListener* listener)
{
    listener->handleMovePlayer(shared_from_this());
}

void ServerboundMovePlayerPacket::read(DataInputStream* input)
{
    char flags = input->readByte();
    onGround = (flags & 1) != 0;
    flying = (flags & 2) != 0;
}

void ServerboundMovePlayerPacket::write(DataOutputStream* output)
{
    unsigned char flags = onGround | (flying ? 2 : 0);
    output->writeByte(flags);
}

int ServerboundMovePlayerPacket::getEstimatedSize()
{
    return 1;
}

bool ServerboundMovePlayerPacket::canBeInvalidated()
{
    return true;
}

bool ServerboundMovePlayerPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return true;
}

double ServerboundMovePlayerPacket::getX(double defaultValue)
{
    return hasPositionValue ? x : defaultValue;
}

double ServerboundMovePlayerPacket::getY(double defaultValue)
{
    return hasPositionValue ? y : defaultValue;
}

double ServerboundMovePlayerPacket::getZ(double defaultValue)
{
    return hasPositionValue ? z : defaultValue;
}

float ServerboundMovePlayerPacket::getYRot(float defaultValue)
{
    return hasRotationValue ? yRot : defaultValue;
}

float ServerboundMovePlayerPacket::getXRot(float defaultValue)
{
    return hasRotationValue ? xRot : defaultValue;
}

bool ServerboundMovePlayerPacket::isOnGround()
{
    return onGround;
}

bool ServerboundMovePlayerPacket::hasPosition()
{
    return hasPositionValue;
}

bool ServerboundMovePlayerPacket::hasRotation()
{
    return hasRotationValue;
}

void ServerboundMovePlayerPacket::setHasPosition(bool value)
{
    hasPositionValue = value;
}

bool ServerboundMovePlayerPacket::isFlying()
{
    return flying;
}

void ServerboundMovePlayerPacket::setIsFlying(bool value)
{
    flying = value;
}

int ServerboundMovePlayerPacket::getPacketId()
{
    return 10;
}

boost::shared_ptr<Packet> ServerboundMovePlayerPacket::create()
{
    return boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket());
}

ServerboundMovePlayerPacket::Pos::Pos() : ServerboundMovePlayerPacket()
{
    hasPositionValue = true;
}

ServerboundMovePlayerPacket::Pos::Pos(double x, double y, double unused, double z, bool onGround, bool isFlying)
    : ServerboundMovePlayerPacket()
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->onGround = onGround;
    this->flying = isFlying;
    hasPositionValue = true;
}

void ServerboundMovePlayerPacket::Pos::read(DataInputStream* input)
{
    x = input->readDouble();
    y = input->readDouble();
    z = input->readDouble();
    ServerboundMovePlayerPacket::read(input);
}

void ServerboundMovePlayerPacket::Pos::write(DataOutputStream* output)
{
    output->writeDouble(x);
    output->writeDouble(y);
    output->writeDouble(z);
    ServerboundMovePlayerPacket::write(output);
}

int ServerboundMovePlayerPacket::Pos::getEstimatedSize()
{
    return 33;
}

int ServerboundMovePlayerPacket::Pos::getPacketId()
{
    return 11;
}

boost::shared_ptr<Packet> ServerboundMovePlayerPacket::Pos::create()
{
    return boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket::Pos());
}

ServerboundMovePlayerPacket::PosRot::PosRot() : ServerboundMovePlayerPacket()
{
    hasRotationValue = true;
    hasPositionValue = true;
}

ServerboundMovePlayerPacket::PosRot::PosRot(double x, double y, double z, float yRot, float xRot, bool onGround,
                                            bool isFlying)
    : ServerboundMovePlayerPacket()
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->yRot = yRot;
    this->xRot = xRot;
    this->onGround = onGround;
    this->flying = isFlying;
    hasPositionValue = true;
    hasRotationValue = true;
}

void ServerboundMovePlayerPacket::PosRot::read(DataInputStream* input)
{
    x = input->readDouble();
    y = input->readDouble();
    z = input->readDouble();
    yRot = input->readFloat();
    xRot = input->readFloat();
    ServerboundMovePlayerPacket::read(input);
}

void ServerboundMovePlayerPacket::PosRot::write(DataOutputStream* output)
{
    output->writeDouble(x);
    output->writeDouble(y);
    output->writeDouble(z);
    output->writeFloat(yRot);
    output->writeFloat(xRot);
    ServerboundMovePlayerPacket::write(output);
}

int ServerboundMovePlayerPacket::PosRot::getEstimatedSize()
{
    return 41;
}

int ServerboundMovePlayerPacket::PosRot::getPacketId()
{
    return 13;
}

boost::shared_ptr<Packet> ServerboundMovePlayerPacket::PosRot::create()
{
    return boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket::PosRot());
}

ServerboundMovePlayerPacket::Rot::Rot() : ServerboundMovePlayerPacket()
{
    hasRotationValue = true;
}

ServerboundMovePlayerPacket::Rot::Rot(float yRot, float xRot, bool onGround, bool isFlying)
    : ServerboundMovePlayerPacket()
{
    this->yRot = yRot;
    this->xRot = xRot;
    this->onGround = onGround;
    this->flying = isFlying;
    hasRotationValue = true;
}

void ServerboundMovePlayerPacket::Rot::read(DataInputStream* input)
{
    yRot = input->readFloat();
    xRot = input->readFloat();
    ServerboundMovePlayerPacket::read(input);
}

void ServerboundMovePlayerPacket::Rot::write(DataOutputStream* output)
{
    output->writeFloat(yRot);
    output->writeFloat(xRot);
    ServerboundMovePlayerPacket::write(output);
}

int ServerboundMovePlayerPacket::Rot::getEstimatedSize()
{
    return 9;
}

int ServerboundMovePlayerPacket::Rot::getPacketId()
{
    return 12;
}

boost::shared_ptr<Packet> ServerboundMovePlayerPacket::Rot::create()
{
    return boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket::Rot());
}
