#ifndef GAME_CORE_NOTWINNERCONDITION_HPP
#define GAME_CORE_NOTWINNERCONDITION_HPP

#include "game/core/WinnerCondition.hpp"

class GameMedal;
class GameStats;
class StatsUID;

class NotWinnerCondition : public WinnerCondition
{
public:
    NotWinnerCondition();
    virtual ~NotWinnerCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
