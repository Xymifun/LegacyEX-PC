#include "game/core/KillsWithBowStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

KillsWithBowStatTracker::KillsWithBowStatTracker(const StatsUID& statsUID) : StatTracker<unsigned int>(statsUID, 0)
{
}

KillsWithBowStatTracker::~KillsWithBowStatTracker()
{
}

void KillsWithBowStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && damageEvent->mFatal
        && damageEvent->mDamageSource == Scoreboard::DAMAGE_SOURCE_ARROW)
    {
        ++mValue;
    }
}
