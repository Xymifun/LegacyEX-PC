#include "game/minigame/ConsolationMedal.hpp"

ConsolationMedal::ConsolationMedal(int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, false, false)
{
}

bool ConsolationMedal::CanAward(const StatsUID& uid, GameStats* gameStats) const
{
    (void)uid;
    (void)gameStats;
    return true;
}
