#include "game/core/TimeSpentCondition.hpp"

#include <algorithm>
#include <climits>

#include "game/world/generation/PlayerStats.hpp"

TimeSpentCondition::TimeSpentCondition(float target, eTimeSpentCategory category, GameMedal::EOperator op,
                                       bool bestOnly)
    : MultiConditionMedal::MultiCondition(target, op, bestOnly),
      mCategory(category)
{
}

TimeSpentCondition::~TimeSpentCondition()
{
}

float TimeSpentCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    const bool highest = mOperator == GameMedal::OPERATOR_AT_LEAST;
    unsigned int score = highest ? 0 : UINT_MAX;
    for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (mCategory == eTimeSpentCategory_BetweenCheckpoints)
        {
            int previousTime = 0;
            for (unsigned int checkpoint = 0; checkpoint < static_cast<unsigned int>(roundStats->mLastCheckpointID + 1);
                 ++checkpoint)
            {
                if (roundStats->mCheckpointTimes.find(checkpoint) != roundStats->mCheckpointTimes.end())
                {
                    const unsigned int split = roundStats->mCheckpointTimes.at(checkpoint) - previousTime;
                    previousTime = roundStats->mCheckpointTimes.at(checkpoint);
                    if (highest)
                        score = std::max(score, split);
                    else
                        score = std::min(score, split);
                }
            }
        }
        else
        {
            const unsigned int timeSpent = roundStats->mTimeSpent[mCategory];
            if (highest)
                score = std::max(score, timeSpent);
            else
                score = std::min(score, timeSpent);
        }
    }
    return static_cast<float>(score);
}
