#ifndef GAME_ENTITY_MOB_MOBEFFECTS_HPP
#define GAME_ENTITY_MOB_MOBEFFECTS_HPP

#include <string>

class MobEffect;

class MobEffects
{
public:
    static void staticCtor();

    static MobEffect* MOVEMENT_SPEED;
    static MobEffect* MOVEMENT_SLOWDOWN;
    static MobEffect* DIG_SPEED;
    static MobEffect* DIG_SLOWDOWN;
    static MobEffect* DAMAGE_BOOST;
    static MobEffect* HEAL;
    static MobEffect* HARM;
    static MobEffect* JUMP;
    static MobEffect* CONFUSION;
    static MobEffect* REGENERATION;
    static MobEffect* DAMAGE_RESISTANCE;
    static MobEffect* FIRE_RESISTANCE;
    static MobEffect* WATER_BREATHING;
    static MobEffect* INVISIBILITY;
    static MobEffect* BLINDNESS;
    static MobEffect* NIGHT_VISION;
    static MobEffect* HUNGER;
    static MobEffect* WEAKNESS;
    static MobEffect* POISON;
    static MobEffect* WITHER;
    static MobEffect* HEALTH_BOOST;
    static MobEffect* ABSORPTION;
    static MobEffect* SATURATION;
    static MobEffect* GLOWING;
    static MobEffect* LEVITATION;
    static MobEffect* LUCK;
    static MobEffect* UNLUCK;
    static MobEffect* CONDUIT_POWER;
    static MobEffect* SLOW_FALLING;

private:
    static MobEffect* get(const std::wstring& name);
};

#endif
