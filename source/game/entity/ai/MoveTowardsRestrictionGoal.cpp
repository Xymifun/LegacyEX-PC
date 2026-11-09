#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"

MoveTowardsRestrictionGoal::MoveTowardsRestrictionGoal(PathfinderMob* mob, double speedModifier)
    : mMob(mob),
      mSpeedModifier(speedModifier)
{
    mWantedZ = 0.0;
    mWantedY = 0.0;
    mWantedX = 0.0;
    setRequiredControlFlags(1);
}

MoveTowardsRestrictionGoal::~MoveTowardsRestrictionGoal()
{
}

bool MoveTowardsRestrictionGoal::canUse()
{
    if (mMob->isWithinRestriction())
    {
        return false;
    }

    BlockPos restrictionCenter = mMob->getRestrictCenter();
    Vec3* position = RandomPos::getPosTowards(
        boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 16, 7,
        Vec3::newTemp((double)restrictionCenter.x, (double)restrictionCenter.y, (double)restrictionCenter.z));
    if (position == NULL)
    {
        return false;
    }

    mWantedX = position->x;
    mWantedY = position->y;
    mWantedZ = position->z;
    return true;
}

bool MoveTowardsRestrictionGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

void MoveTowardsRestrictionGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}
