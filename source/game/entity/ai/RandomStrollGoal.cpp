#include "game/entity/ai/RandomStrollGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"

RandomStrollGoal::RandomStrollGoal(PathfinderMob* mob, double speedModifier)
{
    _init(mob, speedModifier, 120);
}

RandomStrollGoal::RandomStrollGoal(PathfinderMob* mob, double speedModifier, int interval)
{
    _init(mob, speedModifier, interval);
}

void RandomStrollGoal::_init(PathfinderMob* mob, double speedModifier, int interval)
{
    mForceTrigger = false;
    mWantedX = mWantedY = mWantedZ = 0.0;
    mMob = mob;
    mSpeedModifier = speedModifier;
    mInterval = interval;
    setRequiredControlFlags(1);
}

bool RandomStrollGoal::canUse()
{
    if (mMob->followsLeader())
    {
        return false;
    }
    bool extraWandering = mMob->isExtraWanderingEnabled();
    if (!mForceTrigger && !extraWandering)
    {
        if (mMob->getNoActionTime() >= 100)
        {
            return false;
        }
        if (mMob->getRandom()->nextInt(mInterval) != 0)
        {
            return false;
        }
    }
    Vec3* target = getPosition();
    if (target == NULL)
    {
        return false;
    }
    mWantedX = target->x;
    mWantedY = target->y;
    mWantedZ = target->z;
    mForceTrigger = false;
    return true;
}

bool RandomStrollGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

void RandomStrollGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

Vec3* RandomStrollGoal::getPosition()
{
    return RandomPos::getPos(boost::static_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 10, 7,
                             mMob->getWanderingQuadrant());
}

void RandomStrollGoal::trigger()
{
    mForceTrigger = true;
}

void RandomStrollGoal::setInterval(int interval)
{
    mInterval = interval;
}
