#ifndef GAME_ENTITY_MOB_ENTITYDAMAGESOURCE_HPP
#define GAME_ENTITY_MOB_ENTITYDAMAGESOURCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"

class Entity;

class EntityDamageSource : public DamageSource
{
public:
    EntityDamageSource(ClientboundChatPacket::EChatPacketMessage message,
                       ClientboundChatPacket::EChatPacketMessage deathMessage, boost::shared_ptr<Entity> entity);

    bool isThorns() const;
    virtual boost::shared_ptr<Entity> getEntity() const;
    virtual boost::shared_ptr<ClientboundChatPacket> getDeathMessagePacket(
        boost::shared_ptr<LivingEntity> entity) const;
    virtual bool scalesWithDifficulty() const;
    virtual Vec3* getSourcePosition() const;
    virtual DamageSource* copy() const;
    EntityDamageSource* setThorns();

protected:
    boost::shared_ptr<Entity> entity;
    bool thorns;
};

#endif
