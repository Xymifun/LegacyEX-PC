#include "game/world/generation/LastPlayerDamageStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

LastPlayerDamageStatTracker::LastPlayerDamageStatTracker(const StatsUID& statsUID)
    : StatTracker<const DamageEvent*>(statsUID, NULL)
{
}

LastPlayerDamageStatTracker::~LastPlayerDamageStatTracker()
{
    delete mValue;
}

void LastPlayerDamageStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mVictim))
    {
        delete mValue;
        mValue = new DamageEvent(*damageEvent);
    }
}
