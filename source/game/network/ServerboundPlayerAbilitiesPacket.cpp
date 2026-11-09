#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"

#include "game/core/Abilities.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundPlayerAbilitiesPacket::ServerboundPlayerAbilitiesPacket()
{
    invulnerable = false;
    flying = false;
    mayFly = false;
    instabuild = false;
    flyingSpeed = 0.0f;
    walkingSpeed = 0.0f;
}

ServerboundPlayerAbilitiesPacket::ServerboundPlayerAbilitiesPacket(Abilities* abilities)
{
    setInvulnerable(abilities->mInvulnerable);
    setFlying(abilities->mFlying);
    setCanFly(abilities->mMayfly);
    setInstabuild(abilities->mInstabuild);
    setFlyingSpeed(abilities->getFlyingSpeed());
    setWalkingSpeed(abilities->getWalkingSpeed());
}

int ServerboundPlayerAbilitiesPacket::getPacketId()
{
    return 227;
}

void ServerboundPlayerAbilitiesPacket::read(DataInputStream* input)
{
    unsigned char flags = input->readByte();
    setInvulnerable((flags & 1) > 0);
    setFlying((flags & 2) > 0);
    setCanFly((flags & 4) > 0);
    setInstabuild((flags & 8) > 0);
    setFlyingSpeed(input->readFloat());
    setWalkingSpeed(input->readFloat());
}

void ServerboundPlayerAbilitiesPacket::write(DataOutputStream* output)
{
    unsigned char flags = 0;
    if (isInvulnerable())
    {
        flags = (unsigned char)(flags | 1);
    }
    if (isFlying())
    {
        flags = (unsigned char)(flags | 2);
    }
    if (canFly())
    {
        flags = (unsigned char)(flags | 4);
    }
    if (canInstabuild())
    {
        flags = (unsigned char)(flags | 8);
    }
    output->writeByte(flags);
    output->writeFloat(flyingSpeed);
    output->writeFloat(walkingSpeed);
}

void ServerboundPlayerAbilitiesPacket::handle(PacketListener* listener)
{
    listener->handlePlayerAbilities(shared_from_this());
}

bool ServerboundPlayerAbilitiesPacket::isInvulnerable()
{
    return invulnerable;
}

void ServerboundPlayerAbilitiesPacket::setInvulnerable(bool invulnerable)
{
    this->invulnerable = invulnerable;
}

bool ServerboundPlayerAbilitiesPacket::isFlying()
{
    return flying;
}

void ServerboundPlayerAbilitiesPacket::setFlying(bool flying)
{
    this->flying = flying;
}

bool ServerboundPlayerAbilitiesPacket::canFly()
{
    return mayFly;
}

void ServerboundPlayerAbilitiesPacket::setCanFly(bool mayFly)
{
    this->mayFly = mayFly;
}

bool ServerboundPlayerAbilitiesPacket::canInstabuild()
{
    return instabuild;
}

float ServerboundPlayerAbilitiesPacket::getFlyingSpeed()
{
    return flyingSpeed;
}

float ServerboundPlayerAbilitiesPacket::getWalkingSpeed()
{
    return walkingSpeed;
}

void ServerboundPlayerAbilitiesPacket::setInstabuild(bool instabuild)
{
    this->instabuild = instabuild;
}

void ServerboundPlayerAbilitiesPacket::setFlyingSpeed(float flyingSpeed)
{
    this->flyingSpeed = flyingSpeed;
}

void ServerboundPlayerAbilitiesPacket::setWalkingSpeed(float walkingSpeed)
{
    this->walkingSpeed = walkingSpeed;
}
