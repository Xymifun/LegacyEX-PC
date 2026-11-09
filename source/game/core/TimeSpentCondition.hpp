#ifndef GAME_CORE_TIMESPENTCONDITION_HPP
#define GAME_CORE_TIMESPENTCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"
#include "game/world/generation/PlayerStats.hpp"

class TimeSpentCondition : public MultiConditionMedal::MultiCondition
{
public:
    TimeSpentCondition(float target, eTimeSpentCategory category, GameMedal::EOperator op, bool bestOnly);

    virtual ~TimeSpentCondition();
    virtual float getScore(const GameMedal* medal, PlayerStats* playerStats) const;

private:
    eTimeSpentCategory mCategory;
};

#endif
