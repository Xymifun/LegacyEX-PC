#include "game/minigame/AmphibiousMedal.hpp"
#include "game/core/AmphibiousKillStatTracker.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

float AmphibiousMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->mAmphibiousKills->getValue()));
}
