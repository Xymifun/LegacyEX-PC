#ifndef GAME_ENTITY_MOB_INDIRECTENTITYDAMAGESOURCE_HPP
#define GAME_ENTITY_MOB_INDIRECTENTITYDAMAGESOURCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/EntityDamageSource.hpp"

class DamageSource;
class Entity;

class IndirectEntityDamageSource : public EntityDamageSource
{
public:
    IndirectEntityDamageSource(ClientboundChatPacket::EChatPacketMessage message,
                               ClientboundChatPacket::EChatPacketMessage deathMessage,
                               boost::shared_ptr<Entity> directEntity, boost::shared_ptr<Entity> entity,
                               const DamageSource* baseDamageSource);

    virtual boost::shared_ptr<Entity> getDirectEntity() const;
    virtual boost::shared_ptr<Entity> getEntity() const;
    virtual boost::shared_ptr<ClientboundChatPacket> getDeathMessagePacket(
        boost::shared_ptr<LivingEntity> entity) const;
    virtual const DamageSource* GetBaseDamageSource() const;
    virtual DamageSource* copy() const;

private:
    boost::shared_ptr<Entity> owner;
    const DamageSource* baseDamageSource;
};

#endif
