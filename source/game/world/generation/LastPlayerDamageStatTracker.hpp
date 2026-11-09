#ifndef GAME_WORLD_GENERATION_LASTPLAYERDAMAGESTATTRACKER_HPP
#define GAME_WORLD_GENERATION_LASTPLAYERDAMAGESTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class LastPlayerDamageStatTracker : public StatTracker<const DamageEvent*>
{
public:
    LastPlayerDamageStatTracker(const StatsUID& statsUID);
    virtual ~LastPlayerDamageStatTracker();

    virtual void update(const DamageEvent* damageEvent);
};

#endif
