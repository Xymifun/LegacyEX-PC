#ifndef GAME_CORE_IMPROVEDCONDITION_HPP
#define GAME_CORE_IMPROVEDCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class ImprovedCondition : public MultiConditionMedal::MultiCondition
{
public:
    ImprovedCondition(float target, GameMedal::EOperator op);
    virtual ~ImprovedCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
