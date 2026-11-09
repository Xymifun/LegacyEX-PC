#ifndef GAME_CORE_SNIPERSTATTRACKER_HPP
#define GAME_CORE_SNIPERSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class SniperStatTracker : public StatTracker<double>
{
public:
    SniperStatTracker(const StatsUID& statsUID);
    virtual ~SniperStatTracker();

    virtual void update(const DamageEvent* damageEvent);
};

#endif
