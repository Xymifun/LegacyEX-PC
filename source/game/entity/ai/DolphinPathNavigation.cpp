#include "game/entity/ai/DolphinPathNavigation.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Dolphin.hpp"
#include "game/core/Path.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

DolphinPathNavigation::DolphinPathNavigation(Dolphin* dolphin, Level* level)
    : WaterBoundPathNavigation(dolphin, level, true, true)
{
}

bool DolphinPathNavigation::canUpdatePath()
{
    return true;
}

bool DolphinPathNavigation::isStableDestination(const BlockPos& pos)
{
    if (mMob->isInWater())
        return WaterBoundPathNavigation::isStableDestination(pos);
    return !mLevel->getBlockState(pos)->isSolid();
}

void DolphinPathNavigation::updatePath()
{
    if (mMob->isInWater())
    {
        WaterBoundPathNavigation::updatePath();
        return;
    }

    Vec3* mobPos = getTempMobPos();
    const double waypointHeight = 0.6;
    const double waypointDistance = 0.36;

    for (int index = mPath->getIndex(); index < mPath->getSize(); ++index)
    {
        Vec3* pathPos = mPath->getPos(mMob->shared_from_this(), index);
        Vec3* horizontalPos = Vec3::newTemp(mobPos->x, pathPos->y, mobPos->z);
        if (horizontalPos->distanceToSqr(pathPos) < waypointDistance
            && std::abs(mobPos->y - pathPos->y) < waypointHeight)
            mPath->setIndex(index + 1);
    }

    int width = static_cast<int>(std::ceil(mMob->mBbWidth));
    int height = static_cast<int>(mMob->mBbHeight) + 1;
    for (int best = std::min(mPath->getIndex() + 6, mPath->getSize() - 1); best > mPath->getIndex(); --best)
    {
        Vec3* pathPos = mPath->getPos(mMob->shared_from_this(), best);
        if (pathPos->distanceToSqr(mobPos) <= 36.0 && canMoveDirectly(mobPos, pathPos, width, height, width))
        {
            mPath->setIndex(best);
            break;
        }
    }

    doStuckDetection(mobPos);
}

Path* DolphinPathNavigation::createPath(const BlockPos& pos)
{
    if (mMob->isInWater())
        return PathNavigation::createPath(pos);
    return PathNavigation::createDirectPath(pos);
}
