#ifndef GAME_CORE_DAMAGEDEALTSTATTRACKER_HPP
#define GAME_CORE_DAMAGEDEALTSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class DamageDealtStatTracker : public StatTracker<float>
{
public:
    DamageDealtStatTracker(const StatsUID& statsUID);
    virtual ~DamageDealtStatTracker();

    virtual void update(const DamageEvent* damageEvent);
};

#endif
