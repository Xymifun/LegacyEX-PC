#include "game/entity/mob/IndirectEntityDamageSource.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/world/generation/Player.hpp"

IndirectEntityDamageSource::IndirectEntityDamageSource(ClientboundChatPacket::EChatPacketMessage messageValue,
                                                       ClientboundChatPacket::EChatPacketMessage deathMessageValue,
                                                       boost::shared_ptr<Entity> directEntityValue,
                                                       boost::shared_ptr<Entity> entityValue,
                                                       const DamageSource* baseDamageSourceValue)
    : EntityDamageSource(messageValue, deathMessageValue, directEntityValue)
{
    owner = entityValue;
    baseDamageSource = baseDamageSourceValue;
}

boost::shared_ptr<Entity> IndirectEntityDamageSource::getDirectEntity() const
{
    return entity;
}

boost::shared_ptr<Entity> IndirectEntityDamageSource::getEntity() const
{
    return owner;
}

boost::shared_ptr<ClientboundChatPacket> IndirectEntityDamageSource::getDeathMessagePacket(
    boost::shared_ptr<LivingEntity> victim) const
{
    not_null_ptr<ItemInstance> item = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                                          ? boost::static_pointer_cast<LivingEntity>(entity)->getMainHandItem()
                                          : ItemInstance::EMPTY;
    std::wstring attackerName = L"";

    boost::shared_ptr<Entity> attacker;
    if (owner != NULL)
    {
        attacker = owner;
        if (owner->GetType() == EntityType::SERVER_PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(owner);
            if (player)
                attackerName = player->getDisplayName();
        }
    }
    else
    {
        attacker = entity;
    }

    if (!item->isEmpty() && item->hasCustomHoverName())
    {
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
            victim->getNetworkName(), deathMessage, attacker, attackerName, item->getHoverName()));
    }
    else
    {
        return boost::shared_ptr<ClientboundChatPacket>(
            new ClientboundChatPacket(victim->getNetworkName(), message, attacker, attackerName));
    }
}

const DamageSource* IndirectEntityDamageSource::GetBaseDamageSource() const
{
    if (baseDamageSource == NULL)
        return DamageSource::GetBaseDamageSource();
    return baseDamageSource;
}

DamageSource* IndirectEntityDamageSource::copy() const
{
    return new IndirectEntityDamageSource(*this);
}
