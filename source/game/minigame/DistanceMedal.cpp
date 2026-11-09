#include "game/minigame/DistanceMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float DistanceMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalWalkDistance())
                               + static_cast<float>(playerStats->getTotalSprintDistance())
                               + static_cast<float>(playerStats->getTotalSneakDistance())
                               + static_cast<float>(playerStats->getTotalSwimDistance())
                               + static_cast<float>(playerStats->getTotalClimbDistance())
                               + static_cast<float>(playerStats->getTotalUnderwaterDistance()));
}
