#ifndef GAME_CORE_WONBYAMOUNTCONDITION_HPP
#define GAME_CORE_WONBYAMOUNTCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class WonByAmountCondition : public MultiConditionMedal::MultiCondition
{
public:
    WonByAmountCondition(float target, GameMedal::EOperator op);
    virtual ~WonByAmountCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
