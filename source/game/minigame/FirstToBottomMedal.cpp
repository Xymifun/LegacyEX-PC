#include "game/minigame/FirstToBottomMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

float FirstToBottomMedal::GetScore(PlayerStats* playerStats) const
{
    int count = 0;
    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mDeathLayer != PlayerStats::RoundStats::LAYERS_UNUSED && roundStats->mLanded)
            ++count;
    }
    return static_cast<float>(count);
}
