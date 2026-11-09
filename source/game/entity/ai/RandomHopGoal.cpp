#include "game/entity/ai/RandomHopGoal.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"

RandomHopGoal::~RandomHopGoal()
{
}

void RandomHopGoal::_init(PathfinderMob* mob, double speedModifier, int interval)
{
    mMob = mob;
    mWantedX = 0.0;
    mWantedY = 0.0;
    mWantedZ = 0.0;
    mSpeedModifier = speedModifier;
    mInterval = interval;

    setRequiredControlFlags(1);
}

bool RandomHopGoal::canUse()
{
    if (mMob->getNoActionTime() >= 100)
    {
        return false;
    }

    if (mMob->getRandom()->nextInt(mInterval) != 0)
    {
        return false;
    }

    Vec3* target = RandomPos::getPos(boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 10, 7, -1);

    if (target == NULL)
    {
        return false;
    }

    mWantedX = target->x;
    mWantedY = target->y;
    mWantedZ = target->z;

    return true;
}

bool RandomHopGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

void RandomHopGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}
