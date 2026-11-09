#include "game/network/ClientboundMoveEntityPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include <boost/pointer_cast.hpp>

void __debugbreak();

ClientboundMoveEntityPacket::ClientboundMoveEntityPacket()
{
    mEntityId = -1;
    mHasRotation = false;
    mXa = 0;
    mYa = 0;
    mZa = 0;
    mYRot = 0;
    mXRot = 0;
}

ClientboundMoveEntityPacket::ClientboundMoveEntityPacket(int entityId)
{
    mEntityId = entityId;
    mHasRotation = false;
    mXa = 0;
    mYa = 0;
    mZa = 0;
    mYRot = 0;
    mXRot = 0;
}

int ClientboundMoveEntityPacket::getPacketId()
{
    return 30;
}

void ClientboundMoveEntityPacket::read(DataInputStream* input)
{
    mEntityId = input->readShort();
}

void ClientboundMoveEntityPacket::write(DataOutputStream* output)
{
#ifdef MINECRAFT_PC
    if (static_cast<unsigned int>(mEntityId) >= 0x8000)
#else
    if (static_cast<unsigned int>(mEntityId) >= 0x800)
#endif
    {
        __debugbreak();
    }
    output->writeShort(static_cast<short>(mEntityId));
}

void ClientboundMoveEntityPacket::handle(PacketListener* listener)
{
    listener->handleMoveEntity(shared_from_this());
}

int ClientboundMoveEntityPacket::getEstimatedSize()
{
    return 2;
}

bool ClientboundMoveEntityPacket::canBeInvalidated()
{
    return true;
}
bool ClientboundMoveEntityPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundMoveEntityPacket> other = boost::dynamic_pointer_cast<ClientboundMoveEntityPacket>(
        packet);
    return other && other->mEntityId == mEntityId;
}
int ClientboundMoveEntityPacket::getEntityId()
{
    return mEntityId;
}
int ClientboundMoveEntityPacket::getXa()
{
    return mXa;
}
int ClientboundMoveEntityPacket::getYa()
{
    return mYa;
}
int ClientboundMoveEntityPacket::getZa()
{
    return mZa;
}
unsigned char ClientboundMoveEntityPacket::getyRot()
{
    return mYRot;
}
unsigned char ClientboundMoveEntityPacket::getxRot()
{
    return mXRot;
}
bool ClientboundMoveEntityPacket::hasRotation()
{
    return mHasRotation;
}
bool ClientboundMoveEntityPacket::isOnGround()
{
    return mOnGround;
}

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundMoveEntityPacket());
}

ClientboundMoveEntityPacket::Pos::Pos() : ClientboundMoveEntityPacket()
{
}

ClientboundMoveEntityPacket::Pos::Pos(int entityId, int xa, int ya, int za, bool onGround)
    : ClientboundMoveEntityPacket(entityId)
{
    mXa = xa;
    mYa = ya;
    mZa = za;
    mOnGround = onGround;
}

int ClientboundMoveEntityPacket::Pos::getPacketId()
{
    return 31;
}
void ClientboundMoveEntityPacket::Pos::read(DataInputStream* input)
{
    ClientboundMoveEntityPacket::read(input);
    unsigned int packed = input->readInt();
    mXa = unpackSigned(packed, 22, 10);
    mYa = unpackSigned(packed, 12, 10);
    mZa = unpackSigned(packed, 2, 10);
    mOnGround = unpackUnsigned(packed, 0, 1) != 0;
}
void ClientboundMoveEntityPacket::Pos::write(DataOutputStream* output)
{
    ClientboundMoveEntityPacket::write(output);
    unsigned int packed = packInt(mXa, 22, 10);
    packed |= packInt(mYa, 12, 10);
    packed |= packInt(mZa, 2, 10);
    packed |= packInt(mOnGround, 0, 1);
    output->writeInt(packed);
}
int ClientboundMoveEntityPacket::Pos::getEstimatedSize()
{
    return 5;
}

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::Pos::create()
{
    return boost::shared_ptr<Packet>(new Pos());
}

ClientboundMoveEntityPacket::Rot::Rot() : ClientboundMoveEntityPacket()
{
    mHasRotation = true;
}

ClientboundMoveEntityPacket::Rot::Rot(int entityId, unsigned char yRot, unsigned char xRot, bool onGround)
    : ClientboundMoveEntityPacket(entityId)
{
    mYRot = yRot;
    mXRot = xRot;
    mHasRotation = true;
    mOnGround = onGround;
}

int ClientboundMoveEntityPacket::Rot::getPacketId()
{
    return 32;
}

void ClientboundMoveEntityPacket::Rot::read(DataInputStream* input)
{
    ClientboundMoveEntityPacket::read(input);
    mYRot = input->readByte();
    mXRot = input->readByte();
    mOnGround = input->readBoolean();
}

void ClientboundMoveEntityPacket::Rot::write(DataOutputStream* output)
{
    ClientboundMoveEntityPacket::write(output);
    output->writeByte(mYRot);
    output->writeByte(mXRot);
    output->writeBoolean(mOnGround);
}

int ClientboundMoveEntityPacket::Rot::getEstimatedSize()
{
    return 4;
}

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::Rot::create()
{
    return boost::shared_ptr<Packet>(new Rot());
}

ClientboundMoveEntityPacket::PosRot::PosRot() : ClientboundMoveEntityPacket()
{
    mHasRotation = true;
}
ClientboundMoveEntityPacket::PosRot::PosRot(int entityId, int xa, int ya, int za, unsigned char yRot,
                                            unsigned char xRot, bool onGround)
    : ClientboundMoveEntityPacket(entityId)
{
    mXa = xa;
    mYa = ya;
    mZa = za;
    mYRot = yRot;
    mXRot = xRot;
    mOnGround = onGround;
    mHasRotation = true;
}
int ClientboundMoveEntityPacket::PosRot::getPacketId()
{
    return 33;
}
void ClientboundMoveEntityPacket::PosRot::read(DataInputStream* input)
{
    ClientboundMoveEntityPacket::read(input);
    unsigned int packed = input->readInt();
    mXa = unpackSigned(packed, 22, 10);
    mYa = unpackSigned(packed, 12, 10);
    mZa = unpackSigned(packed, 2, 10);
    mOnGround = unpackUnsigned(packed, 0, 1) != 0;
    mYRot = input->readByte();
    mXRot = input->readByte();
}
void ClientboundMoveEntityPacket::PosRot::write(DataOutputStream* output)
{
    ClientboundMoveEntityPacket::write(output);
    unsigned int packed = packInt(mXa, 22, 10);
    packed |= packInt(mYa, 12, 10);
    packed |= packInt(mZa, 2, 10);
    packed |= packInt(mOnGround, 0, 1);
    output->writeInt(packed);
    output->writeByte(mYRot);
    output->writeByte(mXRot);
}
int ClientboundMoveEntityPacket::PosRot::getEstimatedSize()
{
    return 7;
}

boost::shared_ptr<Packet> ClientboundMoveEntityPacket::PosRot::create()
{
    return boost::shared_ptr<Packet>(new PosRot());
}
