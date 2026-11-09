#include "game/core/TotalThermalsEnteredCondition.hpp"

#include "game/world/generation/PlayerStats.hpp"

TotalThermalsEnteredCondition::TotalThermalsEnteredCondition(float target, GameMedal::EOperator op, bool bestOnly)
    : MultiConditionMedal::MultiCondition(target, op, bestOnly)
{
}

TotalThermalsEnteredCondition::~TotalThermalsEnteredCondition()
{
}

float TotalThermalsEnteredCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->getTotalThermalsEntered())));
}
