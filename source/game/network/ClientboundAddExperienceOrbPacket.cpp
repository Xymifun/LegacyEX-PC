#include "game/network/ClientboundAddExperienceOrbPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/network/PacketListener.hpp"

ClientboundAddExperienceOrbPacket::ClientboundAddExperienceOrbPacket()
{
    mId = 0;
    mZ = 0;
    mY = 0;
    mX = 0;
    mValue = 0;
}

ClientboundAddExperienceOrbPacket::ClientboundAddExperienceOrbPacket(boost::shared_ptr<ExperienceOrb> entity, int, int,
                                                                     int x, int y, int z)
{
    mId = entity->getId();
    mX = x;
    mY = y;
    mZ = z;
    mValue = entity->getValue();
}

void ClientboundAddExperienceOrbPacket::read(DataInputStream* input)
{
    mId = input->readInt();
    mX = input->readInt();
    mY = input->readInt();
    mZ = input->readInt();
    mValue = input->readShort();
}

void ClientboundAddExperienceOrbPacket::write(DataOutputStream* output)
{
    output->writeInt(mId);
    output->writeInt(mX);
    output->writeInt(mY);
    output->writeInt(mZ);
    output->writeShort((short)mValue);
}

void ClientboundAddExperienceOrbPacket::handle(PacketListener* listener)
{
    listener->handleAddExperienceOrb(shared_from_this());
}

int ClientboundAddExperienceOrbPacket::getEstimatedSize()
{
    return 18;
}

int ClientboundAddExperienceOrbPacket::getId()
{
    return mId;
}

int ClientboundAddExperienceOrbPacket::getX()
{
    return mX;
}

int ClientboundAddExperienceOrbPacket::getY()
{
    return mY;
}

int ClientboundAddExperienceOrbPacket::getZ()
{
    return mZ;
}

int ClientboundAddExperienceOrbPacket::getValue()
{
    return mValue;
}

int ClientboundAddExperienceOrbPacket::getPacketId()
{
    return 26;
}
