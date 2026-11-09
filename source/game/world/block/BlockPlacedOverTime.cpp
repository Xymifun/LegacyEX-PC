#include "game/world/block/BlockPlacedOverTime.hpp"

#include <map>

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

float countEmpty(const std::map<int, int>& blocksPlaced, int firstRound, int lastRound)
{
    float score = 0.0f;
    int direction = lastRound < firstRound ? -1 : 1;

    while (firstRound != lastRound)
    {
        std::map<int, int>::const_iterator found = blocksPlaced.find(firstRound);
        if (found != blocksPlaced.end() && found->second != 0)
        {
            break;
        }

        firstRound += direction;
        score += 10.0f;
    }

    return score;
}

BlockPlacedOverTime::BlockPlacedOverTime(int name, float target, unsigned int weight, ScoreFunction& scoreFunction,
                                         GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false),
      mScoreFunction(&scoreFunction)
{
}

float BlockPlacedOverTime::START_DELAY(int lastRound, const std::map<int, int>& blocksPlaced)
{
    return countEmpty(blocksPlaced, 0, lastRound);
}

float BlockPlacedOverTime::TIME_LEFTOVER(int lastRound, const std::map<int, int>& blocksPlaced)
{
    return countEmpty(blocksPlaced, lastRound, 0);
}

float BlockPlacedOverTime::IN_FIRST_10SECS(int lastRound, const std::map<int, int>& blocksPlaced)
{
    std::map<int, int>::const_iterator found = blocksPlaced.find(0);
    if (found == blocksPlaced.end())
    {
        return 0.0f;
    }

    return (float)found->second;
}

float BlockPlacedOverTime::IN_LAST_10SECS(int lastRound, const std::map<int, int>& blocksPlaced)
{
    std::map<int, int>::const_iterator found = blocksPlaced.find(lastRound - 1);
    if (found == blocksPlaced.end())
    {
        return 0.0f;
    }

    return (float)found->second;
}

float BlockPlacedOverTime::GetScore(PlayerStats* playerStats) const
{
    float score = 0.0f;

    for (unsigned int round = 0; round < (unsigned int)playerStats->getTotalRounds(); round++)
    {
        if (!playerStats->hasRoundStats(round))
        {
            continue;
        }

        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        int roundTime = getGameStats()->GetRoundTime();
        score += (*mScoreFunction)(roundTime / 10 - 1, roundStats->mBlocksPlaced.mByTime);
    }

    return score;
}
