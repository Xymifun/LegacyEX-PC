#include "game/entity/ai/TurtlePanicGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Turtle.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/world/block/BlockPos.hpp"

TurtlePanicGoal::TurtlePanicGoal(Turtle* turtle, double speedModifier)
    : PanicGoal(static_cast<PathfinderMob*>(turtle), speedModifier)
{
}

TurtlePanicGoal::~TurtlePanicGoal()
{
}

bool TurtlePanicGoal::canUse()
{
    if (mMob->getLastHurtByMob().get() == NULL && !mMob->isOnFire())
    {
        return false;
    }

    BlockPos* water = lookForWater(mMob->mLevel, mMob, 7, 4);
    if (water != NULL)
    {
        mWantedX = static_cast<double>(water->x);
        mWantedY = static_cast<double>(water->y);
        mWantedZ = static_cast<double>(water->z);
        return true;
    }

    return findRandomPosition();
}
