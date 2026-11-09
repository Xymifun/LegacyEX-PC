#include "game/entity/mob/AttackDamageMobEffect.hpp"

AttackDamageMobEffect::AttackDamageMobEffect(bool harmful, eMinecraftColour color, double damage)
    : MobEffect(harmful, color),
      damage(damage)
{
}

double AttackDamageMobEffect::getAttributeModifierValue(int amplifier, AttributeModifier*)
{
    return damage * (double)(amplifier + 1);
}
