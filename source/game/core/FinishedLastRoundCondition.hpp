#ifndef GAME_CORE_FINISHEDLASTROUNDCONDITION_HPP
#define GAME_CORE_FINISHEDLASTROUNDCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class FinishedLastRoundCondition : public MultiConditionMedal::MultiCondition
{
public:
    FinishedLastRoundCondition();
    virtual ~FinishedLastRoundCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
