#include "game/core/SniperStatTracker.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

#include <algorithm>

SniperStatTracker::SniperStatTracker(const StatsUID& statsUID) : StatTracker<double>(statsUID, 0.0)
{
}

SniperStatTracker::~SniperStatTracker()
{
}

void SniperStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && damageEvent->mWeapon.GetItem() == Items::bow)
    {
        mValue = std::max(mValue, static_cast<double>(damageEvent->mDistance));
    }
}
