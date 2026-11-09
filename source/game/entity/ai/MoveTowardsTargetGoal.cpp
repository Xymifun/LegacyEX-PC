#include "game/entity/ai/MoveTowardsTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/RandomPos.hpp"

MoveTowardsTargetGoal::MoveTowardsTargetGoal(PathfinderMob* mob, double speedModifier, float within)
{
    mWantedX = mWantedY = mWantedZ = 0.0;
    mMob = mob;
    mSpeedModifier = speedModifier;
    mWithin = within;
    setRequiredControlFlags(1);
}

bool MoveTowardsTargetGoal::canUse()
{
    mTarget = boost::weak_ptr<LivingEntity>(mMob->getTarget());
    if (!mTarget.lock())
    {
        return false;
    }

    if (mTarget.lock()->distanceToSqr(mMob->shared_from_this()) > (double)(mWithin * mWithin))
    {
        return false;
    }

    Vec3* position = RandomPos::getPosTowards(
        boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 16, 7,
        Vec3::newTemp(mTarget.lock()->mX, mTarget.lock()->mY, mTarget.lock()->mZ));
    if (position == NULL)
    {
        return false;
    }

    mWantedX = position->x;
    mWantedY = position->y;
    mWantedZ = position->z;
    return true;
}

bool MoveTowardsTargetGoal::canContinueToUse()
{
    return mTarget.lock() && !mMob->getNavigation()->isDone() && mTarget.lock()->isAlive()
           && mTarget.lock()->distanceToSqr(mMob->shared_from_this()) < (double)(mWithin * mWithin);
}

void MoveTowardsTargetGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

void MoveTowardsTargetGoal::stop()
{
    mTarget = boost::weak_ptr<Mob>();
}
