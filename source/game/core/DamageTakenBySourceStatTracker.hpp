#ifndef GAME_CORE_DAMAGETAKENBYSOURCESTATTRACKER_HPP
#define GAME_CORE_DAMAGETAKENBYSOURCESTATTRACKER_HPP

#include "game/core/Scoreboard.hpp"
#include "game/core/StatTracker.hpp"

class DamageEvent;

class DamageTakenBySourceStatTracker : public StatTracker<float>
{
public:
    DamageTakenBySourceStatTracker(const StatsUID& statsUID, Scoreboard::EDamageSource source);
    virtual ~DamageTakenBySourceStatTracker();
    virtual void update(const DamageEvent* damageEvent);

private:
    Scoreboard::EDamageSource mDamageSource;
};

#endif
