#include "game/core/HasSpawnedAtAllCheckpointsCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

HasSpawnedAtAllCheckpointsCondition::HasSpawnedAtAllCheckpointsCondition()
    : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

HasSpawnedAtAllCheckpointsCondition::~HasSpawnedAtAllCheckpointsCondition()
{
}

bool HasSpawnedAtAllCheckpointsCondition::canAward(const GameMedal* medal, const StatsUID& uid,
                                                   GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
    {
        return false;
    }

    return playerStats->hasSpawnedAtAllCheckpointsInOneRound();
}
