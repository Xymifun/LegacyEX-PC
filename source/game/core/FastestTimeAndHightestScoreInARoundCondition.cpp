#include "game/core/FastestTimeAndHightestScoreInARoundCondition.hpp"

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/GameStats.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/StatsUID.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerStats.hpp"

FastestTimeAndHightestScoreInARoundCondition::FastestTimeAndHightestScoreInARoundCondition()
    : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

FastestTimeAndHightestScoreInARoundCondition::~FastestTimeAndHightestScoreInARoundCondition()
{
}

bool FastestTimeAndHightestScoreInARoundCondition::canAward(const GameMedal* medal, const StatsUID& uid,
                                                            GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
    {
        return false;
    }

    GameStats::StatsMap* allPlayerStats = gameStats->GetPlayerStats();
    const unsigned int roundCount = gMinecraftServer->mMasterGameMode->mGameStats.mCurrentRound;

    for (unsigned int round = 0; round <= roundCount; ++round)
    {
        if (playerStats->getReachedEnd(round))
        {
            unsigned int fastestTime = 0x7fffffffU;
            unsigned int highestScore = 0;
            for (GameStats::StatsMap::iterator it = allPlayerStats->begin(); it != allPlayerStats->end(); ++it)
            {
                PlayerStats* otherStats = it->second;
                const unsigned int time = otherStats->getLifeSpan(round);
                const unsigned int score = otherStats->getPoints(round);
                if (otherStats->getReachedEnd(round) && time < fastestTime)
                {
                    fastestTime = time;
                }
                if (score > highestScore)
                {
                    highestScore = score;
                }
            }

            bool fastest = playerStats->getLifeSpan(round) <= fastestTime;
            bool highest = playerStats->getPoints(round) >= highestScore;
            if (fastest && highest)
            {
                return true;
            }
        }
    }

    return false;
}
