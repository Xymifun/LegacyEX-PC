#ifndef GAME_CORE_TOTALKILLSSTATTRACKER_HPP
#define GAME_CORE_TOTALKILLSSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class TotalKillsStatTracker : public StatTracker<unsigned int>
{
public:
    TotalKillsStatTracker(const StatsUID& statsUID);
    virtual ~TotalKillsStatTracker();
    virtual void update(const DamageEvent* damageEvent);
};

#endif
