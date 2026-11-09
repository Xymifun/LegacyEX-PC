#ifndef GAME_CORE_LASTINFIRSTROUNDCONDITION_HPP
#define GAME_CORE_LASTINFIRSTROUNDCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class LastInFirstRoundCondition : public MultiConditionMedal::MultiCondition
{
public:
    LastInFirstRoundCondition();
    virtual ~LastInFirstRoundCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
