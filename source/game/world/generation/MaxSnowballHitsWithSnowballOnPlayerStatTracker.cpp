#include "game/world/generation/MaxSnowballHitsWithSnowballOnPlayerStatTracker.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

MaxSnowballHitsWithSnowballOnPlayerStatTracker::MaxSnowballHitsWithSnowballOnPlayerStatTracker(const StatsUID& statsUID)
    : StatTracker<unsigned int>(statsUID, 0)
{
    mSnowballHits[mPlayerUID] = 0;
}

void MaxSnowballHitsWithSnowballOnPlayerStatTracker::update(const DamageEvent* damageEvent)
{
    const StatsUID& victim = damageEvent->mVictim;
    if (isTracking(damageEvent->mAttacker) && damageEvent->mWeapon.mItemId == Item::getId(Items::snowball))
    {
        StatsUID emptyPlayerUID;
        if (victim != emptyPlayerUID)
        {
            boost::unordered_map<StatsUID, unsigned int, boost::hash<StatsUID>, std::equal_to<StatsUID> >::iterator
                found = mSnowballHits.find(victim);
            if (found != mSnowballHits.end())
            {
                ++mSnowballHits[victim];
            }
            else
            {
                mSnowballHits[victim] = 1;
            }

            if (mSnowballHits[mPlayerUID] <= mSnowballHits[victim])
            {
                mValue = mSnowballHits[victim];
                mPlayerUID = victim;
            }
        }
    }
}
