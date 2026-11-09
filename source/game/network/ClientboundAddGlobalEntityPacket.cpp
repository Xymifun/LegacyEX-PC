#include "game/network/ClientboundAddGlobalEntityPacket.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Mth.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"

ClientboundAddGlobalEntityPacket::ClientboundAddGlobalEntityPacket() : mId(-1), mX(0), mY(0), mZ(0), mType(0)
{
}

ClientboundAddGlobalEntityPacket::ClientboundAddGlobalEntityPacket(boost::shared_ptr<Entity> entity)
{
    mId = entity->getId();
    mX = Mth::floor(entity->mX * 32.0);
    mY = Mth::floor(entity->mY * 32.0);
    mZ = Mth::floor(entity->mZ * 32.0);
    mType = entity->instanceof(EntityType::LIGHTNING_BOLT) ? 1 : 0;
}

int ClientboundAddGlobalEntityPacket::getPacketId()
{
    return 0x47;
}

void ClientboundAddGlobalEntityPacket::read(DataInputStream* input)
{
    mId = input->readInt();
    mType = input->readByte();
    mX = input->readInt();
    mY = input->readInt();
    mZ = input->readInt();
}

void ClientboundAddGlobalEntityPacket::write(DataOutputStream* output)
{
    output->writeInt(mId);
    output->writeByte(mType);
    output->writeInt(mX);
    output->writeInt(mY);
    output->writeInt(mZ);
}

void ClientboundAddGlobalEntityPacket::handle(PacketListener* listener)
{
    listener->handleAddGlobalEntity(shared_from_this());
}

int ClientboundAddGlobalEntityPacket::getEstimatedSize()
{
    return 17;
}

int ClientboundAddGlobalEntityPacket::getId()
{
    return mId;
}

int ClientboundAddGlobalEntityPacket::getX()
{
    return mX;
}

int ClientboundAddGlobalEntityPacket::getY()
{
    return mY;
}

int ClientboundAddGlobalEntityPacket::getZ()
{
    return mZ;
}

int ClientboundAddGlobalEntityPacket::getType()
{
    return mType;
}
