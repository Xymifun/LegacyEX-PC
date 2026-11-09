#ifndef GAME_CORE_COMBATRULES_HPP
#define GAME_CORE_COMBATRULES_HPP

class CombatRules
{
public:
    static float getDamageAfterAbsorb(float damage, float armorValue, float armorToughness);
    static float getDamageAfterMagicAbsorb(float damage, float enchantModifier);
};

#endif
