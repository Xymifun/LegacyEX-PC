#include "game/core/WinRoundCountCondition.hpp"

WinRoundCountCondition::~WinRoundCountCondition()
{
}

float WinRoundCountCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return static_cast<float>(static_cast<unsigned int>(playerStats->getTotalRoundsWon()));
}
