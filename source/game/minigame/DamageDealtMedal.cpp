#include "game/minigame/DamageDealtMedal.hpp"
#include "game/core/DamageDealtStatTracker.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

float DamageDealtMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(playerStats->mDamageDealt->getValue());
}

bool DamageDealtMedal::IncludeDamage(const DamageEvent* damageEvent) const
{
    return true;
}
