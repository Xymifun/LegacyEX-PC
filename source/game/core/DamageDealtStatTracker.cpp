#include "game/core/DamageDealtStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

DamageDealtStatTracker::DamageDealtStatTracker(const StatsUID& statsUID) : StatTracker<float>(statsUID, 0.0f)
{
}

DamageDealtStatTracker::~DamageDealtStatTracker()
{
}

void DamageDealtStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && !isTracking(damageEvent->mVictim))
    {
        mValue += damageEvent->mDamage;
    }
}
