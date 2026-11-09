#include "game/core/DamageReceivedStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

DamageReceivedStatTracker::DamageReceivedStatTracker(const StatsUID& statsUID) : StatTracker<float>(statsUID, 0.0f)
{
}

DamageReceivedStatTracker::~DamageReceivedStatTracker()
{
}

void DamageReceivedStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mVictim))
    {
        mValue += damageEvent->mDamage;
    }
}
