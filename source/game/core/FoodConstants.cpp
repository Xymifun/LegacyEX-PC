#include "game/core/FoodConstants.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"

const float FoodConstants::FOOD_SATURATION_POOR = 0.1f;
const float FoodConstants::FOOD_SATURATION_LOW = 0.3f;
const float FoodConstants::FOOD_SATURATION_NORMAL = 0.6f;
const float FoodConstants::FOOD_SATURATION_GOOD = 0.8f;
const float FoodConstants::FOOD_SATURATION_MAX = 1.0f;
const float FoodConstants::FOOD_SATURATION_SUPERNATURAL = 1.2f;

static const float MAX_SATURATION = 20.0f;
static const float START_SATURATION = MAX_SATURATION / 4;
static const float SATURATION_FLOOR = MAX_SATURATION / 8;
static const float EXHAUSTION_JUMP = 0.05f;
static const float EXHAUSTION_SPRINT_JUMP = EXHAUSTION_JUMP * 4;

uint32_t FoodConstants::MaxFood()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getMaxFood(20);
    }
    return 20;
}

float FoodConstants::StartSaturation()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getStartSaturation(START_SATURATION);
    }
    return START_SATURATION;
}

float FoodConstants::ExhaustionHeal()
{
    const float value = 6.0f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionHeal(value);
    }
    return value;
}

float FoodConstants::ExhaustionDrop()
{
    const float value = 4.0f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionDrop(value);
    }
    return value;
}

uint32_t FoodConstants::HealthTickCount()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getHealthTickCount(80);
    }
    return 80;
}

uint32_t FoodConstants::HealthTickCountSaturated()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getHealthTickCountSaturated(10);
    }
    return 10;
}

float FoodConstants::MaxSaturation()
{
    const float value = MAX_SATURATION;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getMaxSaturation(value);
    }
    return value;
}

uint32_t FoodConstants::HealLevel()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getHealLevel(18);
    }
    return 18;
}

uint32_t FoodConstants::StarveLevel()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getStarveLevel(0);
    }
    return 0;
}

uint32_t FoodConstants::SprintLevel()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getSprintLevel(6);
    }
    return 6;
}

float FoodConstants::ExhaustionJump()
{
    const float value = EXHAUSTION_JUMP;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionJump(value);
    }
    return value;
}

float FoodConstants::ExhaustionSprintJump()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSprintJump(EXHAUSTION_SPRINT_JUMP);
    }
    return EXHAUSTION_SPRINT_JUMP;
}

float FoodConstants::ExhaustionMine()
{
    const float value = 0.005f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionMine(value);
    }
    return value;
}

float FoodConstants::ExhaustionAttack()
{
    const float value = 0.1f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionAttack(value);
    }
    return value;
}

float FoodConstants::ExhaustionWalk()
{
    const float value = 0.0f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionWalk(value);
    }
    return value;
}

float FoodConstants::ExhaustionSneak()
{
    const float value = 0.0f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSneak(value);
    }
    return value;
}

float FoodConstants::ExhaustionSprint()
{
    const float value = 0.1f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSprint(value);
    }
    return value;
}

float FoodConstants::ExhaustionSwim()
{
    const float value = 0.01f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionSwim(value);
    }
    return value;
}

float FoodConstants::ExhaustionIdle()
{
    const float value = 0.0f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionIdle(value);
    }
    return value;
}

float FoodConstants::SaturationFloor()
{
    const float value = SATURATION_FLOOR;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getSaturationFloor(value);
    }
    return value;
}

float FoodConstants::ExhaustionDamage()
{
    const float value = 0.1f;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != 0)
    {
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
            return miniGame->getExhaustionDamage(value);
    }
    return value;
}
