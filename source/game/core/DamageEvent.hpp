#ifndef GAME_CORE_DAMAGEEVENT_HPP
#define GAME_CORE_DAMAGEEVENT_HPP

#include <boost/shared_ptr.hpp>
#include "game/core/StatsUID.hpp"
#include "game/core/Scoreboard.hpp"

class Entity;

class DamageEvent
{
public:
    StatsUID mVictim;
    StatsUID mAttacker;
    boost::shared_ptr<Entity> mAttackerEntity;
    float mDistance;
    bool mFatal;
    Scoreboard::EDamageSource mDamageSource;
    float mDamage;
    Scoreboard::Weapon mWeapon;
    bool mInWater;
    float mFallDistance;
    int mFallCount;
    int mTime;
};

#endif
