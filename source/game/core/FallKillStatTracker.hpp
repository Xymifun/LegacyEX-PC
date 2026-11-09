#ifndef GAME_CORE_FALLKILLSTATTRACKER_HPP
#define GAME_CORE_FALLKILLSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class FallKillStatTracker : public StatTracker<unsigned int>
{
public:
    FallKillStatTracker(const StatsUID& statsUID);
    virtual ~FallKillStatTracker();
    virtual void update(const DamageEvent* damageEvent);
};

#endif
