#include "game/world/generation/PlayerCountCondition.hpp"

#include "game/core/GameStats.hpp"

PlayerCountCondition::PlayerCountCondition(float target, GameMedal::EOperator op)
    : MultiConditionMedal::MultiCondition(target, op, false)
{
}

PlayerCountCondition::~PlayerCountCondition()
{
}

bool PlayerCountCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    return GameMedal::defaultCanAward(uid, gameStats, static_cast<float>(gameStats->GetPlayerStats()->size()), mTarget,
                                      mOperator);
}
