#ifndef GAME_CORE_DEADSHOTSTATTRACKER_HPP
#define GAME_CORE_DEADSHOTSTATTRACKER_HPP

#include "game/core/StatTracker.hpp"

class DamageEvent;

class DeadshotStatTracker : public StatTracker<bool>
{
public:
    DeadshotStatTracker(const StatsUID& statsUID);
    virtual ~DeadshotStatTracker();
    virtual void update(const DamageEvent* damageEvent);

private:
    int mFallCount;
};

#endif
