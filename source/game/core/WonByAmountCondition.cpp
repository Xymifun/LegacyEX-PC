#include "game/core/WonByAmountCondition.hpp"

#include <climits>

#include "game/core/GameStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/minigame/MiniGameDef.hpp"

WonByAmountCondition::WonByAmountCondition(float target, GameMedal::EOperator op)
    : MultiConditionMedal::MultiCondition(target, op, false)
{
}

WonByAmountCondition::~WonByAmountCondition()
{
}

bool WonByAmountCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
        return false;

    GameStats::StatsMap* allPlayerStats = gameStats->GetPlayerStats();
    unsigned int playerTime = UINT_MAX;
    unsigned int bestOpponentTime = UINT_MAX;
    for (int round = 0; round <= static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetRoundCount()); ++round)
    {
        for (unsigned int playerRound = 0; playerRound < playerStats->getTotalRounds(); ++playerRound)
        {
            playerTime = playerStats->getRoundStats(playerRound)->mLifeSpan;
            for (GameStats::StatsMap::iterator it = allPlayerStats->begin(); it != allPlayerStats->end(); ++it)
            {
                PlayerStats* opponent = it->second;
                if (opponent->mStatsUID != playerStats->mStatsUID && opponent->hasRoundStats(round)
                    && opponent->getRoundStats(round)->mReachedEnd)
                {
                    unsigned int opponentTime = opponent->getRoundStats(round)->mLifeSpan;
                    if (opponentTime < bestOpponentTime)
                        bestOpponentTime = opponentTime;
                }
            }
        }

        if (playerTime < bestOpponentTime)
        {
            if (mOperator == GameMedal::OPERATOR_AT_MOST)
            {
                float margin = static_cast<float>(bestOpponentTime - playerTime);
                if (margin <= mTarget)
                    return true;
            }
            else if (mOperator == GameMedal::OPERATOR_AT_LEAST)
            {
                float margin = static_cast<float>(bestOpponentTime - playerTime);
                if (margin >= mTarget)
                    return true;
            }
        }
    }
    return false;
}
