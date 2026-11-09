#ifndef GAME_CORE_TOPSPEEDCONDITION_HPP
#define GAME_CORE_TOPSPEEDCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class TopSpeedCondition : public MultiConditionMedal::MultiCondition
{
public:
    TopSpeedCondition(float target, GameMedal::EOperator op);
    virtual ~TopSpeedCondition();

    virtual float getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
