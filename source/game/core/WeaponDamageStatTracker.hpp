#ifndef GAME_CORE_WEAPONDAMAGESTATTRACKER_HPP
#define GAME_CORE_WEAPONDAMAGESTATTRACKER_HPP

#include <map>

#include "game/core/Scoreboard.hpp"
#include "game/core/StatTracker.hpp"

class DamageEvent;

class WeaponDamageStatTracker : public StatTracker<Scoreboard::Weapon>
{
public:
    enum ETrackingType
    {
        TRACKING_RECEIVED,
        TRACKING_DEALT
    };

    WeaponDamageStatTracker(const StatsUID& statsUID, ETrackingType trackingType);
    virtual ~WeaponDamageStatTracker() {}
    virtual void update(const DamageEvent* damageEvent);

private:
    std::map<unsigned int, float> mDamageByWeapon;
    ETrackingType mTrackingType;
};

#endif
