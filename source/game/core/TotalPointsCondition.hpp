#ifndef GAME_CORE_TOTALPOINTSCONDITION_HPP
#define GAME_CORE_TOTALPOINTSCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class TotalPointsCondition : public MultiConditionMedal::MultiCondition
{
public:
    TotalPointsCondition(float target, GameMedal::EOperator op, bool bestOnly);
    virtual ~TotalPointsCondition();

    virtual float getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
