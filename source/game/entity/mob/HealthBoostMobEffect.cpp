#include "game/entity/mob/HealthBoostMobEffect.hpp"

#include "game/core/BaseAttributeMap.hpp"
#include "game/entity/mob/LivingEntity.hpp"

HealthBoostMobEffect::HealthBoostMobEffect(bool harmful, eMinecraftColour color) : MobEffect(harmful, color)
{
}

void HealthBoostMobEffect::removeAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity,
                                                    BaseAttributeMap* attributes, int amplifier)
{
    MobEffect::removeAttributeModifiers(entity, attributes, amplifier);
    if (entity->getHealth() > entity->getMaxHealth())
    {
        entity->setHealth(entity->getMaxHealth());
    }
}
