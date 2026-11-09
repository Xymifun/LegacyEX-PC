#ifndef GAME_WORLD_GENERATION_LASTPLAYERFATALDAMAGESTATTRACKER_HPP
#define GAME_WORLD_GENERATION_LASTPLAYERFATALDAMAGESTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class LastPlayerFatalDamageStatTracker : public StatTracker<const DamageEvent*>
{
public:
    LastPlayerFatalDamageStatTracker(const StatsUID& statsUID);
    virtual ~LastPlayerFatalDamageStatTracker();

    virtual void update(const DamageEvent* damageEvent);
};

#endif
