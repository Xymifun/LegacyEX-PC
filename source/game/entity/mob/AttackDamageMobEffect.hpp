#ifndef GAME_ENTITY_MOB_ATTACKDAMAGEMOBEFFECT_HPP
#define GAME_ENTITY_MOB_ATTACKDAMAGEMOBEFFECT_HPP

#include "game/entity/mob/MobEffect.hpp"

class AttackDamageMobEffect : public MobEffect
{
public:
    AttackDamageMobEffect(bool harmful, eMinecraftColour color, double damage);

    virtual double getAttributeModifierValue(int amplifier, AttributeModifier* modifier);

private:
    double damage;
};

#endif
