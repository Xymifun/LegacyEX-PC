#include "game/world/generation/TimeInLayerCentreMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float TimeInLayerCentreMedal::GetScore(PlayerStats* playerStats) const
{
    int total = 0;

    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        total += playerStats->getRoundStats(round)->mLayerChanges;
    }

    return static_cast<float>(total);
}
