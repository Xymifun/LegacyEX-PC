#include "game/core/LastInFirstRoundCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

LastInFirstRoundCondition::LastInFirstRoundCondition()
    : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

LastInFirstRoundCondition::~LastInFirstRoundCondition()
{
}

bool LastInFirstRoundCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats != NULL && playerStats->getTotalRounds() > 0u)
    {
        int finishPosition = playerStats->getRoundStats(0)->mFinishPosition;
        const int finishPlayerCount = playerStats->getRoundStats(0)->mFinishPlayerCount;
        if (finishPlayerCount > 1)
            return finishPosition == finishPlayerCount - 1;
    }

    return false;
}
