#ifndef GAME_CORE_DAMAGERECEIVEDSTATTRACKER_HPP
#define GAME_CORE_DAMAGERECEIVEDSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class DamageReceivedStatTracker : public StatTracker<float>
{
public:
    DamageReceivedStatTracker(const StatsUID& statsUID);
    virtual ~DamageReceivedStatTracker();

    virtual void update(const DamageEvent* damageEvent);
};

#endif
