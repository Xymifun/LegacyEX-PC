#include "game/core/DamageTakenBySourceStatTracker.hpp"
#include "game/core/DamageEvent.hpp"

DamageTakenBySourceStatTracker::DamageTakenBySourceStatTracker(const StatsUID& statsUID,
                                                               Scoreboard::EDamageSource source)
    : StatTracker<float>(statsUID, 0.0f),
      mDamageSource(source)
{
}

DamageTakenBySourceStatTracker::~DamageTakenBySourceStatTracker()
{
}

void DamageTakenBySourceStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mVictim) && damageEvent->mDamageSource == mDamageSource)
    {
        mValue += damageEvent->mDamage;
    }
}
