#include "game/entity/ai/DrownedPathNavigation.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/HitResult.hpp"
#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/core/PIXNamedEvents.hpp"

DrownedPathNavigation::DrownedPathNavigation(Mob* mob, Level* level)
    : GroundPathNavigation(mob, level, false),
      mAmphibiousNodeEvaluator(mob)
{
    mGroundPathFinder = GroundPathNavigation::createPathFinder();
    mPathFinder = mWaterPathFinder = createPathFinder();
}

DrownedPathNavigation::~DrownedPathNavigation()
{
    mPathFinder = 0;
    delete mWaterPathFinder;
    delete mGroundPathFinder;
}

PathFinder* DrownedPathNavigation::createPathFinder()
{
    mAmphibiousNodeEvaluator.setCanPassDoors(true);
    return new PathFinder(&mAmphibiousNodeEvaluator);
}

Path* DrownedPathNavigation::createPath(const BlockPos& pos)
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (target && (!target->isInWaterOrBubble() || target->mOnGround))
    {
        mPathFinder = mGroundPathFinder;
        Path* path = GroundPathNavigation::createPath(pos);
        if (path)
        {
            mMob->stopSwimming();
            return path;
        }
    }

    if (target && isInLiquid())
    {
        mMob->startSwimming();
        mPathFinder = mWaterPathFinder;
        return PathNavigation::createPath(pos);
    }

    mMob->stopSwimming();
    mPathFinder = mGroundPathFinder;
    return GroundPathNavigation::createPath(pos);
}

bool DrownedPathNavigation::canUpdatePath()
{
    return mMob->mOnGround || isInLiquid();
}

Vec3* DrownedPathNavigation::getTempMobPos()
{
    if (mMob->isSwimming())
    {
        return Vec3::newTemp(mMob->mX, mMob->mBbHeight * 0.5 + mMob->mY, mMob->mZ);
    }

    return GroundPathNavigation::getTempMobPos();
}

void DrownedPathNavigation::tick()
{
    if (mMob->isSwimming())
    {
        ++mTick;
        if (mHasDelayedRecomputation)
        {
            recomputePath();
        }

        if (isDone())
        {
            return;
        }

        if (canUpdatePath())
        {
            PIXBeginNamedEvent(0.0f, "updatePath");
            updatePath();
            PIXEndNamedEvent();
        }
        else if (mPath != 0 && mPath->getIndex() < mPath->getSize())
        {
            Vec3* mobPos = getTempMobPos();
            Vec3* next = mPath->getPos(mMob->shared_from_this(), mPath->getIndex());
            if (mobPos->y > next->y && (!mMob->mOnGround || !isInLiquid())
                && std::floor(mobPos->x) == std::floor(next->x) && std::floor(mobPos->z) == std::floor(next->z))
            {
                mPath->setIndex(mPath->getIndex() + 1);
            }
        }

        debugPathfinding();
        if (isDone())
        {
            return;
        }

        Vec3* current = mPath->currentPos(mMob->shared_from_this());
        if (current == 0)
        {
            return;
        }

        BlockPos below = BlockPos(current).below();
        const AABB* shape = mLevel->getBlockState(below)->getShape(mLevel, below);
        Vec3* target = current->subtract(0.0, 1.0 - shape->mMaxY, 0.0);
        mMob->getMoveControl()->setWantedPosition(target->x, target->y, target->z, mSpeedModifier);
    }
    else
    {
        PathNavigation::tick();
    }
}

void DrownedPathNavigation::updatePath()
{
    if (mMob->isSwimming())
    {
        Vec3* mobPos = getTempMobPos();
        float widthSqr = mMob->mBbWidth * mMob->mBbWidth;
        int lookAhead = 6;
        if (mobPos->distanceToSqr(mPath->getPos(mMob->shared_from_this(), mPath->getIndex())) < widthSqr)
        {
            mPath->next();
        }

        for (int index = std::min(mPath->getIndex() + lookAhead, mPath->getSize() - 1); index > mPath->getIndex();
             --index)
        {
            Vec3* target = mPath->getPos(mMob->shared_from_this(), index);
            if (target->distanceToSqr(mobPos) <= 36.0 && canMoveDirectly(mobPos, target, 0, 0, 0))
            {
                mPath->setIndex(index);
                break;
            }
        }

        doStuckDetection(mobPos);
    }
    else
    {
        PathNavigation::updatePath();
    }
}

bool DrownedPathNavigation::canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth)
{
    if (mMob->isSwimming())
    {
        Vec3* target = Vec3::newTemp(end->x, mMob->mBbHeight * 0.5 + end->y, end->z);
        HitResult* result = mLevel->clip(start, target, false, true, false, false, 0, false);
        return result == 0 || result->mType == HitResult::MISS;
    }

    return GroundPathNavigation::canMoveDirectly(start, end, width, height, depth);
}

bool DrownedPathNavigation::isStableDestination(const BlockPos& pos)
{
    if (mMob->mLevel->isDay() && mMob->mLevel->canSeeSky(pos))
    {
        return false;
    }

    if (mMob->isSwimming())
    {
        return !mLevel->getBlockState(pos)->isSolid();
    }

    return PathNavigation::isStableDestination(pos);
}
