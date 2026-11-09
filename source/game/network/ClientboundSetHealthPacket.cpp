#include "game/network/ClientboundSetHealthPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetHealthPacket::ClientboundSetHealthPacket()
{
    health = 0.0f;
    food = 0;
    saturation = 0.0f;
    damageSource.causeOfDeathId = -1;
    damageSource.challenges = eTelemetryChallenges_None;
}

ClientboundSetHealthPacket::ClientboundSetHealthPacket(float health, int food, float saturation,
                                                       TelemetryDamageSource damageSource)
{
    this->health = health;
    this->food = food;
    this->saturation = saturation;
    this->damageSource = damageSource;
}

int ClientboundSetHealthPacket::getPacketId()
{
    return 8;
}

void ClientboundSetHealthPacket::read(DataInputStream* input)
{
    health = input->readFloat();
    food = input->readVarInt();
    saturation = input->readFloat();
    damageSource.challenges = (ETelemetryChallenges)input->readByte();
    damageSource.causeOfDeathId = input->readVarInt();
    damageSource.playerWeaponId = input->readVarInt();
}

void ClientboundSetHealthPacket::write(DataOutputStream* output)
{
    output->writeFloat(health);
    output->writeVarInt(food);
    output->writeFloat(saturation);
    output->writeByte((unsigned char)damageSource.challenges);
    output->writeVarInt(damageSource.causeOfDeathId);
    output->writeVarInt(damageSource.playerWeaponId);
}

void ClientboundSetHealthPacket::handle(PacketListener* listener)
{
    listener->handleSetHealth(shared_from_this());
}

int ClientboundSetHealthPacket::getEstimatedSize()
{
    return 16;
}

bool ClientboundSetHealthPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetHealthPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return true;
}

float ClientboundSetHealthPacket::getHealth()
{
    return health;
}

int ClientboundSetHealthPacket::getFood()
{
    return food;
}

float ClientboundSetHealthPacket::getSaturation()
{
    return saturation;
}
