#include "game/minigame/LongWayDownMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

namespace
{
const int TOP_LAYER = 0;
}

float LongWayDownMedal::GetScore(PlayerStats* playerStats) const
{
    int count = 0;
    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mDeathLayer == TOP_LAYER && !roundStats->mParticipated)
            ++count;
    }
    return static_cast<float>(count);
}
