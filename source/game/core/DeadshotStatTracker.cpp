#include "game/core/DeadshotStatTracker.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemId.hpp"

DeadshotStatTracker::DeadshotStatTracker(const StatsUID& statsUID) : StatTracker<bool>(statsUID, false), mFallCount(-1)
{
}

DeadshotStatTracker::~DeadshotStatTracker()
{
}

void DeadshotStatTracker::update(const DamageEvent* damageEvent)
{
    if (!Minecraft::InMiniGame(eMiniGameId_Tumble, true))
    {
        return;
    }

    if (isTracking(damageEvent->mAttacker) && damageEvent->mWeapon.mItemId == ItemId::SNOWBALL
        && damageEvent->mVictim != StatsUID::EMPTY_SUID() && damageEvent->mFallDistance > 0.0f)
    {
        mFallCount = damageEvent->mFallCount;
    }

    if (!isTracking(damageEvent->mVictim))
    {
        return;
    }

    if (damageEvent->mFatal && damageEvent->mDamageSource == Scoreboard::DAMAGE_SOURCE_FALL && mFallCount > 0)
    {
        mValue = true;
        return;
    }

    if (damageEvent->mFallCount != mFallCount || damageEvent->mFallDistance == 0.0f)
    {
        mFallCount = -1;
    }
}
