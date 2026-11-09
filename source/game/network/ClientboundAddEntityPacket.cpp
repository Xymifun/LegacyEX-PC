#include "game/network/ClientboundAddEntityPacket.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundAddEntityPacket::ClientboundAddEntityPacket()
{
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data)
{
    _init(entity, type, data, (int)entity->mXp, (int)entity->mYp, (int)entity->mZp,
          (unsigned char)std::floor(entity->mXRot * 256.0f / 360.0f),
          (unsigned char)std::floor(entity->mYRot * 256.0f / 360.0f));
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int yRot, int xRot,
                                                       int xa, int ya, int za)
{
    _init(entity, type, -1, xa, ya, za, yRot, xRot);
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data, int yRot,
                                                       int xRot, int xa, int ya, int za)
{
    _init(entity, type, data, xa, ya, za, yRot, xRot);
}

ClientboundAddEntityPacket::ClientboundAddEntityPacket(boost::shared_ptr<Entity> entity, int type, int data,
                                                       const BlockPos& pos, int yRot, int xRot, int xa, int ya, int za)
{
    _init(entity, type, data, xa, ya, za, yRot, xRot);
    mX = pos.x * 32;
    mY = pos.y * 32;
    mZ = pos.z * 32;
}

void ClientboundAddEntityPacket::_init(boost::shared_ptr<Entity> entity, int type, int data, int xa, int ya, int za,
                                       int yRot, int xRot)
{
    mId = entity->getId();
    mUUID = entity->getUUID();
    mX = (int)std::floor(entity->mX * 32.0);
    mY = (int)std::floor(entity->mY * 32.0);
    mZ = (int)std::floor(entity->mZ * 32.0);
    mType = type;
    mData = data;
    mYRot = (unsigned char)yRot;
    mXRot = (unsigned char)xRot;
    const double minVelocity = -4.0;
    const double maxVelocity = 4.0;
    double xd = entity->mXd;
    if (xd < minVelocity)
        xd = minVelocity;
    else if (xd > maxVelocity)
        xd = maxVelocity;
    mXa = (int)((xd - minVelocity) / (maxVelocity - minVelocity) * 65535.0 + -32768.0);
    double yd = entity->mYd;
    if (yd < minVelocity)
        yd = minVelocity;
    else if (yd > maxVelocity)
        yd = maxVelocity;
    mYa = (int)((yd - minVelocity) / (maxVelocity - minVelocity) * 65535.0 + -32768.0);
    double zd = entity->mZd;
    if (zd < minVelocity)
        zd = minVelocity;
    else if (zd > maxVelocity)
        zd = maxVelocity;
    mZa = (int)((zd - minVelocity) / (maxVelocity - minVelocity) * 65535.0 + -32768.0);
}

void ClientboundAddEntityPacket::read(DataInputStream* input)
{
    mId = input->readShort();
    mUUID = input->readUTF();
    mType = input->readByte();
#ifdef MINECRAFT_PC
    mX = input->readInt();
    mY = input->readInt();
    mZ = input->readInt();
#else
    mX = input->readShort();
    mY = input->readShort();
    mZ = input->readShort();
#endif
    mYRot = input->readByte();
    mXRot = input->readByte();
    mData = input->readInt();
    if (mData >= 0)
    {
        mXa = input->readShort();
        mYa = input->readShort();
        mZa = input->readShort();
    }
    else
    {
        mXa = mYa = mZa = 0;
    }
}

void ClientboundAddEntityPacket::write(DataOutputStream* output)
{
    output->writeShort((short)mId);
    output->writeUTF(mUUID);
    output->writeByte((unsigned char)mType);
#ifdef MINECRAFT_PC
    output->writeInt(mX);
    output->writeInt(mY);
    output->writeInt(mZ);
#else
    output->writeShort((short)mX);
    output->writeShort((short)mY);
    output->writeShort((short)mZ);
#endif
    output->writeByte(mYRot);
    output->writeByte(mXRot);
    output->writeInt(mData);
    if (mData >= 0)
    {
        output->writeShort((short)mXa);
        output->writeShort((short)mYa);
        output->writeShort((short)mZa);
    }
}

void ClientboundAddEntityPacket::handle(PacketListener* listener)
{
    listener->handleAddEntity(shared_from_this());
}

int ClientboundAddEntityPacket::getEstimatedSize()
{
    int size = 0;
    if (mData + 11 >= 0)
        size = 6;
    return size;
}

int ClientboundAddEntityPacket::getPacketId()
{
    return 23;
}

int ClientboundAddEntityPacket::getId()
{
    return mId;
}

std::wstring ClientboundAddEntityPacket::getUUID()
{
    return mUUID;
}

int ClientboundAddEntityPacket::getX()
{
    return mX;
}

int ClientboundAddEntityPacket::getY()
{
    return mY;
}

int ClientboundAddEntityPacket::getZ()
{
    return mZ;
}

double ClientboundAddEntityPacket::getXa()
{
    double value = mXa;
    if (value < -32768.0)
        value = -32768.0;
    else if (value > 32767.0)
        value = 32767.0;
    double t = (value - -32768.0) / 65535.0;
    return t * 8.0 + -4.0;
}

double ClientboundAddEntityPacket::getYa()
{
    double value = mYa;
    if (value < -32768.0)
        value = -32768.0;
    else if (value > 32767.0)
        value = 32767.0;
    double t = (value - -32768.0) / 65535.0;
    return t * 8.0 + -4.0;
}

double ClientboundAddEntityPacket::getZa()
{
    double value = mZa;
    if (value < -32768.0)
        value = -32768.0;
    else if (value > 32767.0)
        value = 32767.0;
    double t = (value - -32768.0) / 65535.0;
    return t * 8.0 + -4.0;
}

unsigned char ClientboundAddEntityPacket::getxRot()
{
    return mXRot;
}

unsigned char ClientboundAddEntityPacket::getyRot()
{
    return mYRot;
}

int ClientboundAddEntityPacket::getType()
{
    return mType;
}

int ClientboundAddEntityPacket::getData()
{
    return mData;
}

void ClientboundAddEntityPacket::setXa(int xa)
{
    mXa = xa;
}

void ClientboundAddEntityPacket::setYa(int ya)
{
    mYa = ya;
}

void ClientboundAddEntityPacket::setZa(int za)
{
    mZa = za;
}

void ClientboundAddEntityPacket::setData(int data)
{
    mData = data;
}
