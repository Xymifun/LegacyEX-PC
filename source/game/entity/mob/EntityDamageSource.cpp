#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/util/Vec3.hpp"

EntityDamageSource::EntityDamageSource(ClientboundChatPacket::EChatPacketMessage messageValue,
                                       ClientboundChatPacket::EChatPacketMessage deathMessageValue,
                                       boost::shared_ptr<Entity> entityValue)
    : DamageSource(messageValue, deathMessageValue)
{
    entity = entityValue;
    thorns = false;
}

bool EntityDamageSource::isThorns() const
{
    return thorns;
}

boost::shared_ptr<Entity> EntityDamageSource::getEntity() const
{
    return entity;
}

boost::shared_ptr<ClientboundChatPacket> EntityDamageSource::getDeathMessagePacket(
    boost::shared_ptr<LivingEntity> victim) const
{
    not_null_ptr<ItemInstance> item = (entity != NULL
                                       && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity.get()->GetType()))
                                          ? boost::static_pointer_cast<LivingEntity>(entity)->getMainHandItem()
                                          : ItemInstance::EMPTY;
    std::wstring attackerName = L"";

    if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, entity->GetType()))
    {
        attackerName = boost::static_pointer_cast<Player>(entity)->getDisplayName();
    }
    else if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
        if (mob->hasCustomName())
            attackerName = mob->getCustomName();
    }

    if (item->hasCustomHoverName())
        return boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
            victim->getNetworkName(), deathMessage, entity, attackerName, item->getHoverName()));
    return boost::shared_ptr<ClientboundChatPacket>(
        new ClientboundChatPacket(victim->getNetworkName(), message, entity, attackerName));
}

bool EntityDamageSource::scalesWithDifficulty() const
{
    return entity && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity.get()->GetType())
           && (entity->GetType() & EntityType::PLAYER) != EntityType::PLAYER;
}

Vec3* EntityDamageSource::getSourcePosition() const
{
    return Vec3::newTemp(entity->mX, entity->mY, entity->mZ);
}

DamageSource* EntityDamageSource::copy() const
{
    return new EntityDamageSource(*this);
}

EntityDamageSource* EntityDamageSource::setThorns()
{
    thorns = true;
    return this;
}
