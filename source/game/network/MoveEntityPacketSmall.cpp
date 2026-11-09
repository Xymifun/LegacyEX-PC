#include "game/network/MoveEntityPacketSmall.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include <boost/pointer_cast.hpp>
#include <cassert>

void __debugbreak();

MoveEntityPacketSmall::MoveEntityPacketSmall()
{
    mEntityId = -1;
    mxRot = 0;
    myRot = 0;
    mYa = 0;
    mOnGround = true;
    mHasRot = 0;
    mZa = 0;
    mXa = 0;
}
MoveEntityPacketSmall::MoveEntityPacketSmall(int entityId)
{
    if (static_cast<unsigned int>(entityId) >= 0x800)
    {
        __debugbreak();
    }
    mEntityId = entityId;
    myRot = 0;
    mOnGround = true;
    mZa = 0;
    mYa = 0;
    mXa = 0;
    mHasRot = 0;
    mxRot = 0;
}
int MoveEntityPacketSmall::getPacketId()
{
    return 162;
}
void MoveEntityPacketSmall::read(DataInputStream* input)
{
    mEntityId = input->readShort();
}
void MoveEntityPacketSmall::write(DataOutputStream* output)
{
    if (static_cast<unsigned int>(mEntityId) >= 0x800)
    {
        __debugbreak();
    }
    output->writeShort(static_cast<short>(mEntityId));
}
void MoveEntityPacketSmall::handle(PacketListener* listener)
{
    listener->handleMoveEntitySmall(shared_from_this());
}
int MoveEntityPacketSmall::getEstimatedSize()
{
    return 3;
}
bool MoveEntityPacketSmall::canBeInvalidated()
{
    return true;
}
bool MoveEntityPacketSmall::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<MoveEntityPacketSmall> other = boost::dynamic_pointer_cast<MoveEntityPacketSmall>(packet);
    return other && other->mEntityId == mEntityId;
}
bool MoveEntityPacketSmall::isOnGround() const
{
    return mOnGround;
}
boost::shared_ptr<Packet> MoveEntityPacketSmall::create()
{
    return boost::shared_ptr<Packet>(new MoveEntityPacketSmall());
}

MoveEntityPacketSmall::Pos::Pos() : MoveEntityPacketSmall()
{
}
MoveEntityPacketSmall::Pos::Pos(int entityId, char xa, char ya, char za, bool onGround)
    : MoveEntityPacketSmall(entityId)
{
    mXa = xa;
    mYa = ya;
    mZa = za;
    mOnGround = onGround;
}
int MoveEntityPacketSmall::Pos::getPacketId()
{
    return 163;
}
void MoveEntityPacketSmall::Pos::read(DataInputStream* input)
{
    unsigned int packed = input->readInt();
    mEntityId = unpackSigned(packed, 20, 12);
    mXa = unpackSigned(packed, 14, 6);
    mYa = unpackSigned(packed, 7, 7);
    mZa = unpackSigned(packed, 1, 6);
    mOnGround = unpackUnsigned(packed, 0, 1) != 0;
}
void MoveEntityPacketSmall::Pos::write(DataOutputStream* output)
{
    unsigned int packed = packInt(mEntityId, 20, 12);
    packed |= packInt(mXa, 14, 6);
    packed |= packInt(mYa, 7, 7);
    packed |= packInt(mZa, 1, 6);
    packed |= packInt(mOnGround, 0, 1);
    output->writeInt(packed);
}
int MoveEntityPacketSmall::Pos::getEstimatedSize()
{
    return 4;
}
boost::shared_ptr<Packet> MoveEntityPacketSmall::Pos::create()
{
    return boost::shared_ptr<Packet>(new Pos());
}

MoveEntityPacketSmall::PosRot::PosRot() : MoveEntityPacketSmall()
{
    mHasRot = 1;
}
MoveEntityPacketSmall::PosRot::PosRot(int entityId, char xa, char ya, char za, char yRot, char xRot, bool onGround)
    : MoveEntityPacketSmall(entityId)
{
    mXa = xa;
    mYa = ya;
    mZa = za;
    myRot = yRot;
    mxRot = xRot;
    mHasRot = 1;
    mOnGround = onGround;
}
int MoveEntityPacketSmall::PosRot::getPacketId()
{
    return 165;
}
void MoveEntityPacketSmall::PosRot::read(DataInputStream* input)
{
    unsigned int packed = input->readInt();
    mEntityId = unpackSigned(packed, 20, 12);
    mXa = unpackSigned(packed, 14, 6);
    mYa = unpackSigned(packed, 7, 7);
    mZa = unpackSigned(packed, 1, 6);
    mOnGround = unpackUnsigned(packed, 0, 1) != 0;
    unsigned short rotation = input->readShort();
    mxRot = unpackSigned(rotation, 8, 8);
    myRot = unpackSigned(rotation, 0, 8);
}
void MoveEntityPacketSmall::PosRot::write(DataOutputStream* output)
{
    unsigned int packed = packInt(mEntityId, 20, 12);
    packed |= packInt(mXa, 14, 6);
    packed |= packInt(mYa, 7, 7);
    packed |= packInt(mZa, 1, 6);
    packed |= packInt(mOnGround, 0, 1);
    output->writeInt(packed);
    unsigned short rotation = packInt(mxRot, 8, 8);
    rotation |= packInt(myRot, 0, 8);
    output->writeShort(static_cast<short>(rotation));
}
int MoveEntityPacketSmall::PosRot::getEstimatedSize()
{
    return 5;
}
boost::shared_ptr<Packet> MoveEntityPacketSmall::PosRot::create()
{
    return boost::shared_ptr<Packet>(new PosRot());
}

MoveEntityPacketSmall::Rot::Rot() : MoveEntityPacketSmall()
{
    mHasRot = 1;
}
MoveEntityPacketSmall::Rot::Rot(int entityId, char yRot, char xRot, bool onGround) : MoveEntityPacketSmall(entityId)
{
    myRot = yRot;
    mxRot = xRot;
    mHasRot = 1;
    mOnGround = onGround;
}
int MoveEntityPacketSmall::Rot::getPacketId()
{
    return 164;
}
void MoveEntityPacketSmall::Rot::read(DataInputStream* input)
{
    unsigned short packed = input->readShort();
    mEntityId = unpackSigned(packed, 4, 12);
    mOnGround = unpackUnsigned(packed, 0, 1) != 0;
    myRot = input->readByte();
}
void MoveEntityPacketSmall::Rot::write(DataOutputStream* output)
{
    if (static_cast<unsigned int>(mEntityId) >= 0x800)
    {
        __debugbreak();
    }
    unsigned short packed = packInt(mEntityId, 4, 12);
    packed |= packInt(mOnGround, 0, 1);
    output->writeShort(static_cast<short>(packed));
    output->writeByte(myRot);
}
int MoveEntityPacketSmall::Rot::getEstimatedSize()
{
    return 3;
}
boost::shared_ptr<Packet> MoveEntityPacketSmall::Rot::create()
{
    return boost::shared_ptr<Packet>(new Rot());
}
