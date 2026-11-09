#include "game/core/ImprovedCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

ImprovedCondition::ImprovedCondition(float target, GameMedal::EOperator op)
    : MultiConditionMedal::MultiCondition(target, op, false)
{
}

ImprovedCondition::~ImprovedCondition()
{
}

bool ImprovedCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    return playerStats != NULL && (float)playerStats->getImprovement() > mTarget;
}
