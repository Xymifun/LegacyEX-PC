#include "game/entity/ai/TurtleTravelGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

TurtleTravelGoal::TurtleTravelGoal(Turtle* turtle, double speedModifier)
    : Goal(),
      mTurtle(turtle),
      mSpeedModifier(speedModifier)
{
}

TurtleTravelGoal::~TurtleTravelGoal()
{
}

bool TurtleTravelGoal::canUse()
{
    return !mTurtle->isGoingHome() && (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg())
           && (mTurtle->isInWater() || mTurtle->isOverWater());
}

void TurtleTravelGoal::start()
{
    Random* random = mTurtle->getRandom();
    int x = random->nextInt(1025);
    int y = random->nextInt(9);
    int z = random->nextInt(1025);
    double targetY = (double)(y - 4);

    if ((double)(mTurtle->mLevel->getSeaLevel() - 1) < targetY + mTurtle->mY)
    {
        targetY = 0.0;
    }

    mTurtle->setTravelPos(
        BlockPos((double)(x - 512) + mTurtle->mX, mTurtle->mY + targetY, (double)(z - 512) + mTurtle->mZ));
    mTurtle->setTravelling(true);
    mReachedPosition = false;
}

void TurtleTravelGoal::tick()
{
    if (!mTurtle->closeToNextPos())
    {
        if (!mTurtle->getNavigation()->isDone())
        {
            return;
        }
    }

    BlockPos travelPos = mTurtle->getTravelPos();
    Vec3* position = RandomPos::getPosTowards(
        boost::static_pointer_cast<PathfinderMob>(mTurtle->shared_from_this()), 16, 3,
        Vec3::newTemp((double)travelPos.x, (double)travelPos.y, (double)travelPos.z), 0.3141592741012573);

    if (position == NULL)
    {
        position = RandomPos::getPosTowards(
            boost::static_pointer_cast<PathfinderMob>(mTurtle->shared_from_this()), 8, 7,
            Vec3::newTemp((double)travelPos.x, (double)travelPos.y, (double)travelPos.z));
        if (position == NULL)
        {
            mReachedPosition = true;
            return;
        }
    }

    int x = (int)position->x;
    int z = (int)position->z;
    if (position->x < (double)x)
    {
        --x;
    }
    if (position->z < (double)z)
    {
        --z;
    }

    BoundingBox box(x - 34, 0, z - 34, x + 34, 0, z + 34);
    if (mTurtle->mLevel->hasChunksAt(box))
    {
        mTurtle->getNavigation()->moveTo(position->x, position->y, position->z, mSpeedModifier);
        return;
    }

    mReachedPosition = true;
}

bool TurtleTravelGoal::canContinueToUse()
{
    BlockPos travelPos = mTurtle->getTravelPos();
    double distance = mTurtle->distanceToSqr(travelPos);
    return distance >= 36.0 && !mReachedPosition && !mTurtle->isGoingHome() && !mTurtle->isInLove()
           && (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg());
}

void TurtleTravelGoal::stop()
{
    mTurtle->setTravelling(false);
    Goal::stop();
}
