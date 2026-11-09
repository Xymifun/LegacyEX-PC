#ifndef GAME_CORE_UNTYPEDSTATTRACKER_HPP
#define GAME_CORE_UNTYPEDSTATTRACKER_HPP

#include "game/core/StatsUID.hpp"

class DamageEvent;

class UntypedStatTracker
{
public:
    UntypedStatTracker(const StatsUID& statsUID);
    virtual ~UntypedStatTracker() {}
    virtual void update(const DamageEvent* damageEvent) = 0;

protected:
    bool isTracking(const StatsUID& statsUID);

    StatsUID mStatsUID;
};

#endif
