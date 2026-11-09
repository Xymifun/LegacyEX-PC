#include "game/core/TopSpeedCondition.hpp"

TopSpeedCondition::TopSpeedCondition(float target, GameMedal::EOperator op)
    : MultiConditionMedal::MultiCondition(target, op, true)
{
}

TopSpeedCondition::~TopSpeedCondition()
{
}

float TopSpeedCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return (double)(float)playerStats->getTopSpeed();
}
