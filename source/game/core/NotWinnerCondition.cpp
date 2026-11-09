#include "game/core/NotWinnerCondition.hpp"

NotWinnerCondition::NotWinnerCondition() : WinnerCondition()
{
}

NotWinnerCondition::~NotWinnerCondition()
{
}

bool NotWinnerCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    return !WinnerCondition::canAward(medal, uid, gameStats);
}
