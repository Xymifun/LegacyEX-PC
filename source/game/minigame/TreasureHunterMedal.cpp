#include "game/minigame/TreasureHunterMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float TreasureHunterMedal::GetScore(PlayerStats* playerStats) const
{
    int totalContainers = 0;

    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        totalContainers += playerStats->getRoundStats(round)->mContainers.size();
    }

    return static_cast<double>(static_cast<float>(totalContainers));
}
