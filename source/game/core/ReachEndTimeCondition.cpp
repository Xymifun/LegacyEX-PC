#include "game/core/ReachEndTimeCondition.hpp"

#include <float.h>

#include "game/world/generation/PlayerStats.hpp"

ReachEndTimeCondition::ReachEndTimeCondition(float target, bool reachedEndOnly, GameMedal::EOperator op)
    : MultiConditionMedal::MultiCondition(target, op, true),
      mReachedEndOnly(reachedEndOnly)
{
}

ReachEndTimeCondition::~ReachEndTimeCondition()
{
}

float ReachEndTimeCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    if (playerStats == NULL)
        return FLT_MAX;
    if (mReachedEndOnly)
        return static_cast<float>(playerStats->getBestLifeSpan(mOperator == GameMedal::OPERATOR_AT_LEAST, false));
    return static_cast<float>(static_cast<unsigned int>(playerStats->getTotalLifeSpan()));
}
