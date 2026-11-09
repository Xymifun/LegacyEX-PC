#ifndef GAME_ENTITY_MOB_HEALTHBOOSTMOBEFFECT_HPP
#define GAME_ENTITY_MOB_HEALTHBOOSTMOBEFFECT_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/MobEffect.hpp"

class BaseAttributeMap;
class LivingEntity;

class HealthBoostMobEffect : public MobEffect
{
public:
    HealthBoostMobEffect(bool harmful, eMinecraftColour color);

    virtual void removeAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity, BaseAttributeMap* attributes,
                                          int amplifier);
};

#endif
