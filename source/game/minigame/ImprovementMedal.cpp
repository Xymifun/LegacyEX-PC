#include "game/minigame/ImprovementMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

ImprovementMedal::ImprovementMedal(int name, float target, unsigned int weight)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
{
}

float ImprovementMedal::GetScore(PlayerStats* playerStats) const
{
    int improvedRounds = 0;
    bool hasOnePointRound = false;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mDeathOrder == 1)
        {
            hasOnePointRound = true;
        }
        if (roundStats->mWon && hasOnePointRound)
        {
            ++improvedRounds;
            hasOnePointRound = false;
        }
    }

    return static_cast<double>(static_cast<float>(improvedRounds));
}
