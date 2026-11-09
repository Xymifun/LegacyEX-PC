#include "game/minigame/FirstToBottomAndWonMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float FirstToBottomAndWonMedal::GetScore(PlayerStats* playerStats) const
{
    int count = 0;
    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mDeathLayer != PlayerStats::RoundStats::LAYERS_UNUSED && roundStats->mLanded
            && roundStats->mWon)
            ++count;
    }
    return static_cast<float>(count);
}
