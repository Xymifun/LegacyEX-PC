#include "game/entity/ai/FlyingPathNavigation.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"

FlyingPathNavigation::FlyingPathNavigation(Mob* mob, Level* level) : PathNavigation(mob, level), mNodeEvaluator(mob)
{
    mPathFinder = createPathFinder();
}

PathFinder* FlyingPathNavigation::createPathFinder()
{
    mNodeEvaluator.setCanPassDoors(true);
    return new PathFinder(&mNodeEvaluator);
}

bool FlyingPathNavigation::canFloat()
{
    return mNodeEvaluator.isCanFloat();
}

bool FlyingPathNavigation::canUpdatePath()
{
    return (canFloat() && isInLiquid()) || !mMob->isPassenger();
}

Vec3* FlyingPathNavigation::getTempMobPos()
{
    return Vec3::newTemp(mMob->mX, mMob->mY, mMob->mZ);
}

Path* FlyingPathNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    return createPath(BlockPos(entity));
}

void FlyingPathNavigation::tick()
{
    ++mTick;
    if (mHasDelayedRecomputation)
        recomputePath();

    if (isDone())
        return;

    if (canUpdatePath())
    {
        updatePath();
    }
    else if (mPath != NULL && mPath->getIndex() < mPath->getSize())
    {
        Vec3* next = mPath->getPos(mMob->shared_from_this(), mPath->getIndex());
        if (Mth::floor(mMob->mX) == Mth::floor(next->x) && Mth::floor(mMob->mY) == Mth::floor(next->y)
            && Mth::floor(mMob->mZ) == Mth::floor(next->z))
            mPath->setIndex(mPath->getIndex() + 1);
    }
    debugPathfinding();

    if (isDone())
        return;

    Vec3* current = mPath->currentPos(mMob->shared_from_this());
    mMob->getMoveControl()->setWantedPosition(current->x, current->y, current->z, mSpeedModifier);
}

bool FlyingPathNavigation::canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth)
{
    int x = Mth::floor(start->x);
    int y = Mth::floor(start->y);
    int z = Mth::floor(start->z);
    double xDelta = end->x - start->x;
    double yDelta = end->y - start->y;
    double zDelta = end->z - start->z;
    double lengthSquared = xDelta * xDelta + yDelta * yDelta + zDelta * zDelta;
    if (lengthSquared < 1.0E-8)
    {
        return false;
    }
    else
    {
        double inverseLength = 1.0 / std::sqrt(lengthSquared);
        xDelta *= inverseLength;
        yDelta *= inverseLength;
        zDelta *= inverseLength;
        double xStepDistance = 1.0 / std::fabs(xDelta);
        double yStepDistance = 1.0 / std::fabs(yDelta);
        double zStepDistance = 1.0 / std::fabs(zDelta);
        double xDistance = x - start->x;
        double yDistance = y - start->y;
        double zDistance = z - start->z;
        if (xDelta >= 0.0)
        {
            xDistance += 1.0;
        }
        if (yDelta >= 0.0)
        {
            yDistance += 1.0;
        }
        if (zDelta >= 0.0)
        {
            zDistance += 1.0;
        }
        xDistance /= xDelta;
        yDistance /= yDelta;
        zDistance /= zDelta;
        int xDirection = xDelta < 0.0 ? -1 : 1;
        int yDirection = yDelta < 0.0 ? -1 : 1;
        int zDirection = zDelta < 0.0 ? -1 : 1;
        int endX = Mth::floor(end->x);
        int endY = Mth::floor(end->y);
        int endZ = Mth::floor(end->z);
        int xRemaining = endX - x;
        int yRemaining = endY - y;
        int zRemaining = endZ - z;
        while (xRemaining * xDirection > 0 || yRemaining * yDirection > 0 || zRemaining * zDirection > 0)
        {
            if (xDistance < zDistance && xDistance <= yDistance)
            {
                xDistance += xStepDistance;
                x += xDirection;
                xRemaining = endX - x;
            }
            else if (yDistance < xDistance && yDistance <= zDistance)
            {
                yDistance += yStepDistance;
                y += yDirection;
                yRemaining = endY - y;
            }
            else
            {
                zDistance += zStepDistance;
                z += zDirection;
                zRemaining = endZ - z;
            }
        }
        return true;
    }
}

void FlyingPathNavigation::setCanOpenDoors(bool value)
{
    mNodeEvaluator.setCanOpenDoors(value);
}

void FlyingPathNavigation::setCanPassDoors(bool value)
{
    mNodeEvaluator.setCanPassDoors(value);
}

void FlyingPathNavigation::setCanFloat(bool value)
{
    mNodeEvaluator.setCanFloat(value);
}

bool FlyingPathNavigation::isStableDestination(const BlockPos& pos)
{
    return mLevel->getBlockState(pos)->isTopSolidBlocking();
}

NodeEvaluator* FlyingPathNavigation::getNodeEvaluator()
{
    return &mNodeEvaluator;
}
