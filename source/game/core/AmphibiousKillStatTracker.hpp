#ifndef GAME_CORE_AMPHIBIOUSKILLSTATTRACKER_HPP
#define GAME_CORE_AMPHIBIOUSKILLSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class AmphibiousKillStatTracker : public StatTracker<unsigned int>
{
public:
    AmphibiousKillStatTracker(const StatsUID& statsUID);
    virtual ~AmphibiousKillStatTracker();
    virtual void update(const DamageEvent* damageEvent);
};

#endif
