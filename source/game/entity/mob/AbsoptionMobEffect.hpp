#ifndef GAME_ENTITY_MOB_ABSOPTIONMOBEFFECT_HPP
#define GAME_ENTITY_MOB_ABSOPTIONMOBEFFECT_HPP

#include "game/entity/mob/MobEffect.hpp"

class AbsoptionMobEffect : public MobEffect
{
public:
    AbsoptionMobEffect(bool harmful, eMinecraftColour color);

    virtual void removeAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity, BaseAttributeMap* attributes,
                                          int amplifier);
    virtual void addAttributeModifiers(const boost::shared_ptr<LivingEntity>& entity, BaseAttributeMap* attributes,
                                       int amplifier);
};

#endif
