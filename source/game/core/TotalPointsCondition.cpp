#include "game/core/TotalPointsCondition.hpp"

TotalPointsCondition::TotalPointsCondition(float target, GameMedal::EOperator op, bool bestOnly)
    : MultiConditionMedal::MultiCondition(target, op, bestOnly)
{
}

TotalPointsCondition::~TotalPointsCondition()
{
}

float TotalPointsCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return (double)(float)(unsigned int)playerStats->getTotalPoints();
}
