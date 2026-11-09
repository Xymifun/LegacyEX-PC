#ifndef GAME_CORE_REACHENDTIMECONDITION_HPP
#define GAME_CORE_REACHENDTIMECONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class ReachEndTimeCondition : public MultiConditionMedal::MultiCondition
{
public:
    ReachEndTimeCondition(float target, bool reachedEndOnly, GameMedal::EOperator op);

    virtual ~ReachEndTimeCondition();
    virtual float getScore(const GameMedal* medal, PlayerStats* playerStats) const;

private:
    bool mReachedEndOnly;
};

#endif
