#include "game/minigame/UnderwaterMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float UnderwaterMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalUnderwaterDistance()));
}
