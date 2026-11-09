#include "game/world/generation/LastPlayerFatalDamageStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

LastPlayerFatalDamageStatTracker::LastPlayerFatalDamageStatTracker(const StatsUID& statsUID)
    : StatTracker<const DamageEvent*>(statsUID, NULL)
{
}

LastPlayerFatalDamageStatTracker::~LastPlayerFatalDamageStatTracker()
{
    delete mValue;
}

void LastPlayerFatalDamageStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mVictim) && damageEvent->mFatal)
    {
        delete mValue;
        mValue = new DamageEvent(*damageEvent);
    }
}
