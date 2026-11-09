#include "game/entity/ai/WaterBoundPathNavigation.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/util/Vec3.hpp"

#include <boost/unordered_set.hpp>

WaterBoundPathNavigation::WaterBoundPathNavigation(Mob* mob, Level* level, bool createPathFinder, bool allowBreaching)
    : PathNavigation(mob, level),
      mNodeEvaluator(allowBreaching)
{
    if (createPathFinder)
        mPathFinder = this->createPathFinder();
}

PathFinder* WaterBoundPathNavigation::createPathFinder()
{
    return new PathFinder(&mNodeEvaluator);
}

bool WaterBoundPathNavigation::canUpdatePath()
{
    return isInLiquid();
}

Vec3* WaterBoundPathNavigation::getTempMobPos()
{
    return Vec3::newTemp(mMob->mX, mMob->mY + (double)mMob->mBbHeight * 0.5, mMob->mZ);
}

void WaterBoundPathNavigation::updatePath()
{
    Vec3* mobPos = getTempMobPos();
    double speed = 0.2;
    float width = mMob->mBbWidth;
    float distance = width > 0.75f ? width * 0.5f : 0.75f - width * 0.5f;
    if (Mth::abs(mMob->mXd) > speed || Mth::abs(mMob->mZd) > speed)
    {
        float motionLength = (float)Mth::sqrt(mMob->mXd * mMob->mXd + mMob->mYd * mMob->mYd + mMob->mZd * mMob->mZd);
        distance = distance * (motionLength * 6.0f);
    }

    Vec3* currentPos = mPath->currentPos(mMob->shared_from_this());

    if ((float)Mth::abs(mMob->mX - (currentPos->x + 0.5)) < distance
        && (float)Mth::abs(mMob->mZ - (currentPos->z + 0.5)) < distance
        && (float)Mth::abs(mMob->mY - currentPos->y) < distance + distance)
        mPath->next();

    int best = mPath->getIndex() + 6;
    int lastIndex = mPath->getSize() - 1;
    if (lastIndex < best)
        best = lastIndex;

    while (mPath->getIndex() < best)
    {
        Vec3* position = mPath->getPos(mMob->shared_from_this(), best);
        if (position->distanceToSqr(mobPos) <= 36.0 && canMoveDirectly(mobPos, position, 0, 0, 0))
        {
            mPath->setIndex(best);
            doStuckDetection(mobPos);
            return;
        }
        --best;
    }
    doStuckDetection(mobPos);
}

bool WaterBoundPathNavigation::canMoveDirectly(Vec3* a, Vec3* b, int x, int y, int z)
{
    HitResult* result = mLevel->clip(a, Vec3::newTemp(b->x, b->y + (double)mMob->mBbHeight * 0.5, b->z), false, true,
                                     false, false, NULL, false);
    bool clear = result == NULL || result->mType == HitResult::MISS;
    delete result;
    return clear;
}

NodeEvaluator* WaterBoundPathNavigation::getNodeEvaluator()
{
    return &mNodeEvaluator;
}

bool WaterBoundPathNavigation::isStableDestination(const BlockPos& pos)
{
    return !mLevel->getBlockState(pos)->isSolid();
}
