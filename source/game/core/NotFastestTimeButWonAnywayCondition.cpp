#include "game/core/NotFastestTimeButWonAnywayCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

NotFastestTimeButWonAnywayCondition::NotFastestTimeButWonAnywayCondition()
    : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

NotFastestTimeButWonAnywayCondition::~NotFastestTimeButWonAnywayCondition()
{
}

bool NotFastestTimeButWonAnywayCondition::canAward(const GameMedal* medal, const StatsUID& uid,
                                                   GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
    {
        return false;
    }

    for (unsigned int round = 0; round < gameStats->mCurrentRound; round++)
    {
        if (playerStats->getRoundWon(round) != 0
            && gameStats->GetShortestLifespan(round) < playerStats->getLifeSpan(round))
        {
            return true;
        }
    }

    return false;
}
