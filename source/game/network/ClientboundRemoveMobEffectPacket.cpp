#include "game/network/ClientboundRemoveMobEffectPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Level.hpp"

ClientboundRemoveMobEffectPacket::ClientboundRemoveMobEffectPacket()
{
}

ClientboundRemoveMobEffectPacket::ClientboundRemoveMobEffectPacket(int entityId, MobEffect* effect)
    : entityId(entityId),
      effect(effect)
{
}

void ClientboundRemoveMobEffectPacket::read(DataInputStream* input)
{
    entityId = input->readVarInt();
    effect = MobEffect::byId(input->readUnsignedByte());
}

void ClientboundRemoveMobEffectPacket::write(DataOutputStream* output)
{
    output->writeVarInt(entityId);
    output->writeByte(MobEffect::getId(effect));
}

void ClientboundRemoveMobEffectPacket::handle(PacketListener* listener)
{
    listener->handleRemoveMobEffect(shared_from_this());
}

int ClientboundRemoveMobEffectPacket::getEstimatedSize()
{
    return 5;
}

boost::shared_ptr<Entity> ClientboundRemoveMobEffectPacket::getEntity(Level* level)
{
    return level->getEntity(entityId);
}

MobEffect* ClientboundRemoveMobEffectPacket::getEffect()
{
    return effect;
}

int ClientboundRemoveMobEffectPacket::getPacketId()
{
    return 42;
}
