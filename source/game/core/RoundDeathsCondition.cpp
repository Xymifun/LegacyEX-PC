#include "game/core/RoundDeathsCondition.hpp"

#include "game/world/generation/PlayerStats.hpp"

RoundDeathsCondition::~RoundDeathsCondition()
{
}

float RoundDeathsCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    int deaths = -1;
    GameMedal::EOperator op = mOperator;
    switch (op)
    {
    case GameMedal::OPERATOR_AT_MOST:
        deaths = playerStats->getDeathsInAnyRound(0);
        break;

    case GameMedal::OPERATOR_AT_LEAST:
        deaths = playerStats->getDeathsInAnyRound(0x7fffffff);
        break;

    case GameMedal::OPERATOR_EQUAL:
        deaths = playerStats->getDeathsInAnyRound(static_cast<int>(mTarget));
        break;
    }

    return static_cast<double>(static_cast<float>(deaths));
}
