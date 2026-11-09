#include "game/entity/projectile/HitsFromProjectileMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float HitsFromProjectileMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<float>(playerStats->mHitsBySnowball);
}
