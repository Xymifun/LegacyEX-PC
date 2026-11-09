#include "game/core/FinishCourseWithDamageTakenCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/world/generation/PlayerStats.hpp"

FinishCourseWithDamageTakenCondition::FinishCourseWithDamageTakenCondition(float target)
    : MultiConditionMedal::MultiCondition(target, GameMedal::OPERATOR_EQUAL, false)
{
}

FinishCourseWithDamageTakenCondition::~FinishCourseWithDamageTakenCondition()
{
}

bool FinishCourseWithDamageTakenCondition::canAward(const GameMedal* medal, const StatsUID& uid,
                                                    GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats != NULL)
    {
        for (unsigned int round = 0; round < static_cast<unsigned int>(playerStats->getTotalRounds()); ++round)
        {
            if (playerStats->getReachedEnd(round) != 0 && playerStats->getDamageTaken(round) == mTarget)
            {
                return true;
            }
        }
    }

    return false;
}
