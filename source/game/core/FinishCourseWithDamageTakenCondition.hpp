#ifndef GAME_CORE_FINISHCOURSEWITHDAMAGETAKENCONDITION_HPP
#define GAME_CORE_FINISHCOURSEWITHDAMAGETAKENCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class GameMedal;
class GameStats;
class PlayerStats;
class StatsUID;

class FinishCourseWithDamageTakenCondition : public MultiConditionMedal::MultiCondition
{
public:
    FinishCourseWithDamageTakenCondition(float target);
    virtual ~FinishCourseWithDamageTakenCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
