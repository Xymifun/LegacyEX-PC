#include "game/entity/mob/AbsoptionMobEffect.hpp"

#include "game/core/BaseAttributeMap.hpp"
#include "game/entity/mob/LivingEntity.hpp"

AbsoptionMobEffect::AbsoptionMobEffect(bool harmful, eMinecraftColour color) : MobEffect(harmful, color)
{
}

void AbsoptionMobEffect::removeAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity,
                                                  BaseAttributeMap* attributes, int amplifier)
{
    entity->setAbsorptionAmount(entity->getAbsorptionAmount() - (float)(4 * (amplifier + 1)));
    MobEffect::removeAttributeModifiers(entity, attributes, amplifier);
}

void AbsoptionMobEffect::addAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity,
                                               BaseAttributeMap* attributes, int amplifier)
{
    entity->setAbsorptionAmount(entity->getAbsorptionAmount() + (float)(4 * (amplifier + 1)));
    MobEffect::addAttributeModifiers(entity, attributes, amplifier);
}
