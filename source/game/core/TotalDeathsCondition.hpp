#ifndef GAME_CORE_TOTALDEATHSCONDITION_HPP
#define GAME_CORE_TOTALDEATHSCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class GameMedal;
class PlayerStats;

class TotalDeathsCondition : public MultiConditionMedal::MultiCondition
{
public:
    TotalDeathsCondition(float target, GameMedal::EOperator op, bool bestOnly);
    virtual ~TotalDeathsCondition();

    virtual float getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
