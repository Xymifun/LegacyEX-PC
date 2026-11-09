#include "game/core/UntypedStatTracker.hpp"

UntypedStatTracker::UntypedStatTracker(const StatsUID& statsUID) : mStatsUID(statsUID)
{
}

bool UntypedStatTracker::isTracking(const StatsUID& statsUID)
{
    return mStatsUID == statsUID;
}
