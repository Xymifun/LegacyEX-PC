#include "game/minigame/GameVoteMedal.hpp"

#include <cstring>
#include <math.h>

float GameVoteMedal::GetScore(PlayerStats* playerStats) const
{
    float roundCount = 0.0f;
    float score = 0.0f;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        float scores[PlayerStats::RoundStats::BUILD_SCORE_COUNT] = {0.0f, 0.0f, 0.0f, 0.0f};
        roundCount += 1.0f;

        float votes[PlayerStats::RoundStats::BUILD_SCORE_COUNT];
        for (int i = 0; i < PlayerStats::RoundStats::BUILD_SCORE_COUNT; ++i)
            votes[i] = roundStats->mScoresReceived[i];

        float totalVotes = votes[0] + votes[1] + votes[2] + votes[3];

        for (int i = 0; i < PlayerStats::RoundStats::BUILD_SCORE_COUNT; ++i)
        {
            float rangeScore = mRanges[i].getScore(votes[i] / totalVotes);
            if (rangeScore >= 0.0f)
            {
                scores[i] = rangeScore;
            }
            else
            {
                std::memset(scores, 0, sizeof(scores));
                break;
            }
        }

        for (int i = 0; i < PlayerStats::RoundStats::BUILD_SCORE_COUNT; ++i)
            score += scores[i];
    }

    return (100.0f * score) / (roundCount + 4.0f);
}

float GameVoteMedal::Range::getScore(float score) const
{
    float difference = fabsf(score - mTarget);
    if (difference < mRange)
    {
        return (double)powf((mRange - difference) / mRange, 1.5f);
    }

    return -1.0;
}
