#include "game/network/ClientboundUpdateMobEffectPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/network/PacketListener.hpp"

ClientboundUpdateMobEffectPacket::ClientboundUpdateMobEffectPacket()
{
    entityId = 0;
    effectId = 0;
    effectAmplifier = 0;
    effectDurationTicks = 0;
    effectFlags = 0;
}

ClientboundUpdateMobEffectPacket::ClientboundUpdateMobEffectPacket(int entityId, MobEffectInstance* effect)
{
    this->entityId = entityId;
    effectId = (unsigned char)MobEffect::getId(effect->getEffect());
    effectAmplifier = (unsigned char)effect->getAmplifier();
    effectDurationTicks = effect->getDuration() <= 32767 ? (short)effect->getDuration() : 32767;

    effectFlags = 0;

    if (effect->isAmbient())
    {
        effectFlags |= 1;
    }

    if (effect->isVisible())
    {
        effectFlags |= 2;
    }
}

void ClientboundUpdateMobEffectPacket::read(DataInputStream* input)
{
    entityId = input->readVarInt();
    effectId = input->readByte();
    effectAmplifier = input->readByte();
    effectDurationTicks = input->readVarInt();
    effectFlags = input->readByte();
}

void ClientboundUpdateMobEffectPacket::write(DataOutputStream* output)
{
    output->writeVarInt(entityId);
    output->writeByte(effectId);
    output->writeByte(effectAmplifier);
    output->writeVarInt(effectDurationTicks);
    output->writeByte(effectFlags);
}

void ClientboundUpdateMobEffectPacket::handle(PacketListener* listener)
{
    listener->handleUpdateMobEffect(shared_from_this());
}

int ClientboundUpdateMobEffectPacket::getEstimatedSize()
{
    return 8;
}

bool ClientboundUpdateMobEffectPacket::canBeInvalidated()
{
    return 1;
}

bool ClientboundUpdateMobEffectPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundUpdateMobEffectPacket> other =
        boost::dynamic_pointer_cast<ClientboundUpdateMobEffectPacket>(packet);

    return other->entityId == entityId && other->effectId == effectId;
}

unsigned int ClientboundUpdateMobEffectPacket::getEntityId()
{
    return entityId;
}

unsigned char ClientboundUpdateMobEffectPacket::getEffectId()
{
    return effectId;
}

unsigned char ClientboundUpdateMobEffectPacket::getEffectAmplifier()
{
    return effectAmplifier;
}

unsigned int ClientboundUpdateMobEffectPacket::getEffectDurationTicks()
{
    return effectDurationTicks;
}

unsigned char ClientboundUpdateMobEffectPacket::isEffectVisible()
{
    return (effectFlags >> 1) & 1;
}

unsigned char ClientboundUpdateMobEffectPacket::isEffectAmbient()
{
    return effectFlags & 1;
}

int ClientboundUpdateMobEffectPacket::getPacketId()
{
    return 41;
}

bool ClientboundUpdateMobEffectPacket::isSuperLongDuration()
{
    return effectDurationTicks == 32767;
}
