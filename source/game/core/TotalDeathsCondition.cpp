#include "game/core/TotalDeathsCondition.hpp"

#include "game/world/generation/PlayerStats.hpp"

TotalDeathsCondition::TotalDeathsCondition(float target, GameMedal::EOperator op, bool bestOnly)
    : MultiConditionMedal::MultiCondition(target, op, bestOnly)
{
}

TotalDeathsCondition::~TotalDeathsCondition()
{
}

float TotalDeathsCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalDeaths()));
}
