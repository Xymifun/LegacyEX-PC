#include "game/minigame/SneakingMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float SneakingMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalSneakDistance()));
}
