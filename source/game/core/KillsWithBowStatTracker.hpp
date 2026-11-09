#ifndef GAME_CORE_KILLSWITHBOWSTATTRACKER_HPP
#define GAME_CORE_KILLSWITHBOWSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class KillsWithBowStatTracker : public StatTracker<unsigned int>
{
public:
    KillsWithBowStatTracker(const StatsUID& statsUID);
    virtual ~KillsWithBowStatTracker();
    virtual void update(const DamageEvent* damageEvent);
};

#endif
