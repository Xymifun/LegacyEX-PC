#include "game/world/block/MoveToBlockGoal.hpp"

#include "game/core/Mth.hpp"
#include "game/core/Node.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

MoveToBlockGoal::MoveToBlockGoal(PathfinderMob* mob, double speedModifier, int searchRange, int verticalSearchStart,
                                 double maxStayDistance)
    : mBlockPos(BlockPos::ZERO)
{
    mMaxStayDuration = 0;
    mNextStartTick = 0;
    mTicksStayingStill = 0;
    mSpeedModifierSquared = maxStayDistance * maxStayDistance;
    mMob = mob;
    mSpeedModifier = speedModifier;
    mReachedTarget = false;
    mSearchRange = searchRange;
    mVerticalSearchStart = verticalSearchStart;
    setRequiredControlFlags(5);
}

bool MoveToBlockGoal::findNearestBlock()
{
    int range = mSearchRange;
    int vertical = mVerticalSearchStart;
    BlockPos origin(mMob);
    BlockPos minPos = origin.offset(-range, -(vertical + 1), -range);
    BlockPos maxPos = origin.offset(range, vertical, range);

    if (!isValidTargetArea(mMob->mLevel, minPos, maxPos))
    {
        return false;
    }

    for (int y = 0; y <= vertical; y = y > 0 ? -y : 1 - y)
    {
        for (int radius = 0; radius < range; ++radius)
        {
            for (int x = 0; x <= radius; x = x > 0 ? -x : 1 - x)
            {
                for (int z = x < radius && x > -radius ? radius : 0; z <= radius; z = z > 0 ? -z : 1 - z)
                {
                    BlockPos candidate = origin.offset(x, y - 1, z);
                    if (mMob->isWithinRestriction(candidate) && isValidTarget(mMob->mLevel, candidate))
                    {
                        mBlockPos = candidate;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool MoveToBlockGoal::canUse()
{
    if (mNextStartTick > 0)
    {
        --mNextStartTick;
        return false;
    }

    mNextStartTick = GetNextStartTick(mMob);
    return findNearestBlock();
}

bool MoveToBlockGoal::canContinueToUse()
{
    return mTicksStayingStill >= -mMaxStayDuration && mTicksStayingStill <= 1200
           && isValidTarget(mMob->mLevel, mBlockPos);
}

void MoveToBlockGoal::start()
{
    mMob->getNavigation()->moveTo((double)mBlockPos.x + 0.5, (double)mBlockPos.y + 1.0, (double)mBlockPos.z + 0.5,
                                  mSpeedModifier);
    mTicksStayingStill = 0;
    mMaxStayDuration = 1200 + mMob->getRandom()->nextInt(1200 + mMob->getRandom()->nextInt(1200));
}

int MoveToBlockGoal::yOff()
{
    return 1;
}

void MoveToBlockGoal::tick()
{
    BlockPos above = mBlockPos.above();
    if (mMob->distanceSqrToBlockPosCenter(above) > mSpeedModifierSquared)
    {
        ++mTicksStayingStill;
        mReachedTarget = false;
        if (mTicksStayingStill % 40 == 0)
        {
            mMob->getNavigation()->moveTo((double)mBlockPos.x + 0.5,
                                          (double)mBlockPos.y + (double)MoveToBlockGoal::yOff(),
                                          (double)mBlockPos.z + 0.5, mSpeedModifier);
        }
    }
    else
    {
        mReachedTarget = true;
        --mTicksStayingStill;
    }
}

bool MoveToBlockGoal::isReachedTarget()
{
    return mReachedTarget;
}

int MoveToBlockGoal::GetNextStartTick(PathfinderMob* mob)
{
    return mob->getRandom()->nextInt(200) + 200;
}

bool MoveToBlockGoal::canReach(const BlockPos& pos)
{
    Vec3* target = Vec3::newTemp((double)pos.x, (double)(pos.y + 1), (double)pos.z);
    Path* path = mMob->getNavigation()->createPath(BlockPos(target));
    if (path == NULL)
    {
        return false;
    }

    Node* last = path->last();
    if (last == NULL)
    {
        return false;
    }

    float halfWidth = mMob->mBbWidth * 0.5f;
    int dx = last->x - Mth::floor(target->x + 0.5 - halfWidth);
    int dz = last->z - Mth::floor(target->z + 0.5 - halfWidth);
    bool result = dx * dx + dz * dz <= 1 && (double)last->y == target->y;
    delete path;
    return result;
}

bool MoveToBlockGoal::isValidTargetArea(Level*, const BlockPos&, const BlockPos&)
{
    return true;
}
