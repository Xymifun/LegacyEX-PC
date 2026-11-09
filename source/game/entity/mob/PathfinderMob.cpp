#include "game/entity/mob/PathfinderMob.hpp"

#include <algorithm>
#include <cmath>

#include "game/core/Path.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/EntityType.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"

PathfinderMob::PathfinderMob(Level* level) : Mob(level)
{
    mPath = NULL;
    mAttackTarget = boost::shared_ptr<Entity>();
    mHoldGround = false;
    mFleeTime = 0;
    mRestrictionRadius = -1.0f;
    mRestrictionCenter = BlockPos::ZERO;
    mRestrictedPathfindingMalus = BlockPathTypes::WATER->getMalus();
}

PathfinderMob::~PathfinderMob()
{
    delete mPath;
}

float PathfinderMob::getWalkTargetValue(const BlockPos&)
{
    return 0.0f;
}

bool PathfinderMob::checkSpawnRules()
{
    bool canSpawn = false;
    if (Mob::checkSpawnRules())
    {
        if (getWalkTargetValue(BlockPos(mX, mBoundingBox->mMinY, mZ)) >= 0.0f)
        {
            canSpawn = true;
        }
    }
    return canSpawn;
}

bool PathfinderMob::isPathFinding()
{
    return !mNavigation->isDone();
}

bool PathfinderMob::isWithinRestriction()
{
    return isWithinRestriction(BlockPos(shared_from_this()));
}

bool PathfinderMob::isWithinRestriction(const BlockPos& pos)
{
    if (mRestrictionRadius != -1.0f)
    {
        return mRestrictionCenter.distSqr(pos) < (double)mRestrictionRadius * (double)mRestrictionRadius;
    }

    return true;
}

void PathfinderMob::restrictTo(const BlockPos& pos, int radius)
{
    mRestrictionCenter = pos;
    mRestrictionRadius = (float)radius;
}

BlockPos PathfinderMob::getRestrictCenter()
{
    return mRestrictionCenter;
}

float PathfinderMob::getRestrictRadius()
{
    return mRestrictionRadius;
}

void PathfinderMob::clearRestriction()
{
    mRestrictionRadius = -1.0f;
}

bool PathfinderMob::hasRestriction()
{
    return mRestrictionRadius != -1.0f;
}

void PathfinderMob::tickLeash()
{
    Entity::tickLeash();
    if (isLeashed() && getLeashHolder() != NULL && getLeashHolder()->mLevel == mLevel)
    {
        boost::shared_ptr<Entity> holder = getLeashHolder();
        restrictTo(BlockPos(static_cast<int>(holder->mX), static_cast<int>(holder->mY), static_cast<int>(holder->mZ)),
                   5);
        float distance = distanceTo(holder);

        boost::shared_ptr<TamableAnimal> tamable = eTYPE_DERIVED_FROM(EntityType::TAMABLE_ANIMAL,
                                                                      shared_from_this()->GetType())
                                                       ? boost::dynamic_pointer_cast<TamableAnimal>(shared_from_this())
                                                       : boost::shared_ptr<TamableAnimal>();

        if (tamable != NULL && tamable->isSitting())
        {
            if (distance > 10.0f)
            {
                dropLeash(true, true);
            }
            return;
        }

        onLeashDistance(distance);
        if (distance > 10.0f)
        {
            dropLeash(true, true);
            mGoalSelector.disableControlFlag(1);
        }
        else if (distance > 6.0f)
        {
            double x = (holder->mX - mX) / distance;
            double y = (holder->mY - mY) / distance;
            double z = (holder->mZ - mZ) / distance;
            mXd += x * std::fabs(x) * 0.4;
            mYd += y * std::fabs(y) * 0.4;
            mZd += z * std::fabs(z) * 0.4;
        }
        else
        {
            mGoalSelector.enableControlFlag(1);
            Vec3* offset = Vec3::newTemp(holder->mX - mX, holder->mY - mY, holder->mZ - mZ)
                               ->normalize()
                               ->scale(std::max(distance - 2.0f, 0.0f));
            getNavigation()->moveTo(mX + offset->x, mY + offset->y, mZ + offset->z, followLeashSpeed());
        }
    }
}

void PathfinderMob::onLeashDistance(float)
{
}

double PathfinderMob::followLeashSpeed()
{
    return 1.0;
}

bool PathfinderMob::couldWander()
{
    return mNoActionTime < 100 || isExtraWanderingEnabled();
}
