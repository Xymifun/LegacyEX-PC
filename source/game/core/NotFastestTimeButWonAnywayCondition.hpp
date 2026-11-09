#ifndef GAME_CORE_NOTFASTESTTIMEBUTWONANYWAYCONDITION_HPP
#define GAME_CORE_NOTFASTESTTIMEBUTWONANYWAYCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class NotFastestTimeButWonAnywayCondition : public MultiConditionMedal::MultiCondition
{
public:
    NotFastestTimeButWonAnywayCondition();
    virtual ~NotFastestTimeButWonAnywayCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
