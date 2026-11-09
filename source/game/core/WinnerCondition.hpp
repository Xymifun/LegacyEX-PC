#ifndef GAME_CORE_WINNERCONDITION_HPP
#define GAME_CORE_WINNERCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class GameMedal;
class GameStats;
class StatsUID;

class WinnerCondition : public MultiConditionMedal::MultiCondition
{
public:
    WinnerCondition();
    virtual ~WinnerCondition() {}

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
