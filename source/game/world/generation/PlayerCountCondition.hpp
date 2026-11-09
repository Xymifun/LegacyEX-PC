#ifndef GAME_WORLD_GENERATION_PLAYERCOUNTCONDITION_HPP
#define GAME_WORLD_GENERATION_PLAYERCOUNTCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class PlayerCountCondition : public MultiConditionMedal::MultiCondition
{
public:
    PlayerCountCondition(float target, GameMedal::EOperator op);
    virtual ~PlayerCountCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
