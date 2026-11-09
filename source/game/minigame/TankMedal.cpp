#include "game/minigame/TankMedal.hpp"
#include "game/core/DamageReceivedStatTracker.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

TankMedal::TankMedal(int name, float target, unsigned int weight)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
{
}

float TankMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(playerStats->mDamageReceived->getValue());
}
