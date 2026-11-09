#include "game/entity/mob/MobEffects.hpp"

#include "game/entity/mob/MobEffect.hpp"
#include "game/util/ResourceLocation.hpp"

MobEffect* MobEffects::MOVEMENT_SPEED = NULL;
MobEffect* MobEffects::MOVEMENT_SLOWDOWN = NULL;
MobEffect* MobEffects::DIG_SPEED = NULL;
MobEffect* MobEffects::DIG_SLOWDOWN = NULL;
MobEffect* MobEffects::DAMAGE_BOOST = NULL;
MobEffect* MobEffects::HEAL = NULL;
MobEffect* MobEffects::HARM = NULL;
MobEffect* MobEffects::JUMP = NULL;
MobEffect* MobEffects::CONFUSION = NULL;
MobEffect* MobEffects::REGENERATION = NULL;
MobEffect* MobEffects::DAMAGE_RESISTANCE = NULL;
MobEffect* MobEffects::FIRE_RESISTANCE = NULL;
MobEffect* MobEffects::WATER_BREATHING = NULL;
MobEffect* MobEffects::INVISIBILITY = NULL;
MobEffect* MobEffects::BLINDNESS = NULL;
MobEffect* MobEffects::NIGHT_VISION = NULL;
MobEffect* MobEffects::HUNGER = NULL;
MobEffect* MobEffects::WEAKNESS = NULL;
MobEffect* MobEffects::POISON = NULL;
MobEffect* MobEffects::WITHER = NULL;
MobEffect* MobEffects::HEALTH_BOOST = NULL;
MobEffect* MobEffects::ABSORPTION = NULL;
MobEffect* MobEffects::SATURATION = NULL;
MobEffect* MobEffects::GLOWING = NULL;
MobEffect* MobEffects::LEVITATION = NULL;
MobEffect* MobEffects::LUCK = NULL;
MobEffect* MobEffects::UNLUCK = NULL;
MobEffect* MobEffects::CONDUIT_POWER = NULL;
MobEffect* MobEffects::SLOW_FALLING = NULL;

MobEffect* MobEffects::get(const std::wstring& name)
{
    return MobEffect::mobEffectRegistry->get(ResourceLocation(name));
}

void MobEffects::staticCtor()
{
    MOVEMENT_SPEED = get(L"speed");
    MOVEMENT_SLOWDOWN = get(L"slowness");
    DIG_SPEED = get(L"haste");
    DIG_SLOWDOWN = get(L"mining_fatigue");
    DAMAGE_BOOST = get(L"strength");
    HEAL = get(L"instant_health");
    HARM = get(L"instant_damage");
    JUMP = get(L"jump_boost");
    CONFUSION = get(L"nausea");
    REGENERATION = get(L"regeneration");
    DAMAGE_RESISTANCE = get(L"resistance");
    FIRE_RESISTANCE = get(L"fire_resistance");
    WATER_BREATHING = get(L"water_breathing");
    INVISIBILITY = get(L"invisibility");
    BLINDNESS = get(L"blindness");
    NIGHT_VISION = get(L"night_vision");
    HUNGER = get(L"hunger");
    WEAKNESS = get(L"weakness");
    POISON = get(L"poison");
    WITHER = get(L"wither");
    HEALTH_BOOST = get(L"health_boost");
    ABSORPTION = get(L"absorption");
    SATURATION = get(L"saturation");
    GLOWING = get(L"glowing");
    LEVITATION = get(L"levitation");
    LUCK = get(L"luck");
    UNLUCK = get(L"unluck");
    CONDUIT_POWER = get(L"conduit_power");
    SLOW_FALLING = get(L"slow_falling");
}
