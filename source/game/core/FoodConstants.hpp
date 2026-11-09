#ifndef GAME_CORE_FOODCONSTANTS_HPP
#define GAME_CORE_FOODCONSTANTS_HPP

#include <stdint.h>

class FoodConstants
{
public:
    static const float FOOD_SATURATION_POOR;
    static const float FOOD_SATURATION_LOW;
    static const float FOOD_SATURATION_NORMAL;
    static const float FOOD_SATURATION_GOOD;
    static const float FOOD_SATURATION_MAX;
    static const float FOOD_SATURATION_SUPERNATURAL;

    static uint32_t MaxFood();
    static float StartSaturation();
    static float ExhaustionHeal();
    static float ExhaustionDrop();
    static uint32_t HealthTickCount();
    static uint32_t HealthTickCountSaturated();
    static float MaxSaturation();
    static uint32_t HealLevel();
    static uint32_t StarveLevel();
    static uint32_t SprintLevel();
    static float ExhaustionJump();
    static float ExhaustionSprintJump();
    static float ExhaustionMine();
    static float ExhaustionAttack();
    static float ExhaustionWalk();
    static float ExhaustionSneak();
    static float ExhaustionSprint();
    static float ExhaustionSwim();
    static float ExhaustionIdle();
    static float SaturationFloor();
    static float ExhaustionDamage();
};

#endif
