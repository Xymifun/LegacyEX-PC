#include "game/minigame/FirstToDieMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

namespace
{
const int FIRST_DEATH = 1;
}

FirstToDieMedal::FirstToDieMedal(int name, float target, unsigned int weight)
    : GameMedal(name, target, weight, GameMedal::OPERATOR_AT_LEAST, true, false)
{
}

float FirstToDieMedal::GetScore(PlayerStats* playerStats) const
{
    float score = 0.0f;
    float roundScore = 10.0f;
    int scoringRounds = 0;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        if (playerStats->getRoundStats(round)->mDeathOrder == FIRST_DEATH)
        {
            ++scoringRounds;
            score += roundScore;
        }

        roundScore *= 0.95f;
    }

    if (scoringRounds >= 2)
    {
        return score;
    }

    return 0.0f;
}
