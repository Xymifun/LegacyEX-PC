#include "game/core/FallKillStatTracker.hpp"
#include "game/core/DamageEvent.hpp"

FallKillStatTracker::FallKillStatTracker(const StatsUID& statsUID) : StatTracker<unsigned int>(statsUID, 0)
{
}

FallKillStatTracker::~FallKillStatTracker()
{
}

void FallKillStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && damageEvent->mFatal && damageEvent->mFallDistance > 1.5)
    {
        mValue += static_cast<unsigned int>(damageEvent->mFallDistance);
    }
}
