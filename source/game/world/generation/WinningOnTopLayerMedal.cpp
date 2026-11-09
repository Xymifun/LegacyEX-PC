#include "game/world/generation/WinningOnTopLayerMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float WinningOnTopLayerMedal::GetScore(PlayerStats* playerStats) const
{
    int count = 0;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mDeathLayer != PlayerStats::RoundStats::LAYERS_UNUSED && roundStats->mWon
            && roundStats->mFinalLayer == 0)
        {
            ++count;
        }
    }

    return static_cast<float>(count);
}
