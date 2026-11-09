#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"

#include "game/core/Abilities.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundPlayerAbilitiesPacket::ClientboundPlayerAbilitiesPacket()
    : mInvulnerable(false),
      mFlying(false),
      mCanFly(false),
      mInstabuild(false),
      mFlyingSpeed(0.0f),
      mWalkingSpeed(0.0f),
      mCanBuild(true),
      mPlayerId(0)
{
}

ClientboundPlayerAbilitiesPacket::ClientboundPlayerAbilitiesPacket(Abilities* abilities, int playerId)
{
    setInvulnerable(abilities->mInvulnerable);
    setFlying(abilities->mFlying);
    setCanFly(abilities->mMayfly);
    setInstabuild(abilities->mInstabuild);
    setFlyingSpeed(abilities->getFlyingSpeed());
    setWalkingSpeed(abilities->getWalkingSpeed());
    setCanBuild(abilities->mMayBuild);
    mPlayerId = playerId;
}

void ClientboundPlayerAbilitiesPacket::setInvulnerable(bool value)
{
    mInvulnerable = value;
}
void ClientboundPlayerAbilitiesPacket::setFlying(bool value)
{
    mFlying = value;
}
void ClientboundPlayerAbilitiesPacket::setCanFly(bool value)
{
    mCanFly = value;
}
void ClientboundPlayerAbilitiesPacket::setInstabuild(bool value)
{
    mInstabuild = value;
}
void ClientboundPlayerAbilitiesPacket::setFlyingSpeed(float value)
{
    mFlyingSpeed = value;
}
void ClientboundPlayerAbilitiesPacket::setWalkingSpeed(float value)
{
    mWalkingSpeed = value;
}
void ClientboundPlayerAbilitiesPacket::setCanBuild(bool value)
{
    mCanBuild = value;
}

void ClientboundPlayerAbilitiesPacket::read(DataInputStream* input)
{
    unsigned char flags = input->readByte();
    setInvulnerable((flags & 1) > 0);
    setFlying((flags & 2) > 0);
    setCanFly((flags & 4) > 0);
    setInstabuild((flags & 8) > 0);
    setCanBuild((flags & 16) > 0);
    setFlyingSpeed(input->readFloat());
    setWalkingSpeed(input->readFloat());
    mPlayerId = input->readInt();
}

bool ClientboundPlayerAbilitiesPacket::isInvulnerable()
{
    return mInvulnerable;
}
bool ClientboundPlayerAbilitiesPacket::isFlying()
{
    return mFlying;
}
bool ClientboundPlayerAbilitiesPacket::canFly()
{
    return mCanFly;
}
bool ClientboundPlayerAbilitiesPacket::canInstabuild()
{
    return mInstabuild;
}
bool ClientboundPlayerAbilitiesPacket::canBuild()
{
    return mCanBuild;
}

void ClientboundPlayerAbilitiesPacket::write(DataOutputStream* output)
{
    unsigned char flags = 0;
    if (isInvulnerable())
        flags |= 1;
    if (isFlying())
        flags |= 2;
    if (canFly())
        flags |= 4;
    if (canInstabuild())
        flags |= 8;
    if (canBuild())
        flags |= 16;
    output->writeByte(flags);
    output->writeFloat(mFlyingSpeed);
    output->writeFloat(mWalkingSpeed);
    output->writeInt(mPlayerId);
}

void ClientboundPlayerAbilitiesPacket::handle(PacketListener* listener)
{
    listener->handlePlayerAbilities(shared_from_this());
}

int ClientboundPlayerAbilitiesPacket::getEstimatedSize()
{
    return 13;
}
float ClientboundPlayerAbilitiesPacket::getFlyingSpeed()
{
    return mFlyingSpeed;
}
float ClientboundPlayerAbilitiesPacket::getWalkingSpeed()
{
    return mWalkingSpeed;
}
bool ClientboundPlayerAbilitiesPacket::canBeInvalidated()
{
    return true;
}
bool ClientboundPlayerAbilitiesPacket::isInvalidatedBy(boost::shared_ptr<Packet>)
{
    return true;
}
int ClientboundPlayerAbilitiesPacket::getPlayerId()
{
    return mPlayerId;
}
int ClientboundPlayerAbilitiesPacket::getPacketId()
{
    return 202;
}
