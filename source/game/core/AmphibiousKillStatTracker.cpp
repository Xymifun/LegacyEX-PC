#include "game/core/AmphibiousKillStatTracker.hpp"
#include "game/core/DamageEvent.hpp"

AmphibiousKillStatTracker::AmphibiousKillStatTracker(const StatsUID& statsUID) : StatTracker<unsigned int>(statsUID, 0)
{
}

AmphibiousKillStatTracker::~AmphibiousKillStatTracker()
{
}

void AmphibiousKillStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && damageEvent->mFatal && damageEvent->mInWater)
    {
        ++mValue;
    }
}
