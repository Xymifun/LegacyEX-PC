#include "game/entity/ai/FollowMobGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPathTypes.hpp"

FollowMobGoal::FollowPredicate::FollowPredicate(Mob* mob) : mMob(mob)
{
}

FollowMobGoal::FollowMobGoal(Mob* mob, double speedModifier, float stopDistance, float areaSize) : mTarget()
{
    mMob = mob;
    mPredicate = new FollowPredicate(mob);
    mSpeedModifier = speedModifier;
    mNavigation = mob->getNavigation();
    mStopDistance = stopDistance;
    mAreaSize = areaSize;
    mTimeToRecalcPath = 0;
    mOldWaterCost = 0.0f;
    setRequiredControlFlags(3);
    dynamic_cast<GroundPathNavigation*>(mob->getNavigation())
        || dynamic_cast<FlyingPathNavigation*>(mob->getNavigation());
}

FollowMobGoal::~FollowMobGoal()
{
    delete mPredicate;
}

bool FollowMobGoal::canUse()
{
    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(
        typeid(Mob), mMob->mBoundingBox->grow((double)mAreaSize), mPredicate, false);

    if (!entities->empty())
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); it++)
        {
            boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
            if (candidate->isInvisible())
                continue;
            mTarget = candidate;
            delete entities;
            return true;
        }
    }

    delete entities;
    return false;
}

bool FollowMobGoal::canContinueToUse()
{
    return mTarget.lock() && !mNavigation->isDone()
           && mMob->distanceToSqr(mTarget.lock()) > (double)(mStopDistance * mStopDistance);
}

void FollowMobGoal::start()
{
    mTimeToRecalcPath = 0;
    mOldWaterCost = mMob->getPathfindingMalus(BlockPathTypes::WATER);
    mMob->setPathfindingMalus(BlockPathTypes::WATER, 0.0f);
}

void FollowMobGoal::stop()
{
    mTarget = boost::weak_ptr<Mob>();
    mNavigation->stop();
    mMob->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
}

void FollowMobGoal::tick()
{
    boost::shared_ptr<Mob> target = mTarget.lock();
    if (!target || mMob->isLeashed())
        return;

    mMob->getLookControl()->setLookAt(target, 10.0f, (float)mMob->getMaxHeadXRot());
    if (--mTimeToRecalcPath > 0)
        return;

    mTimeToRecalcPath = 10;
    double dx = mMob->mX - target->mX;
    double dy = mMob->mY - target->mY;
    double dz = mMob->mZ - target->mZ;
    double distSq = dx * dx + dy * dy + dz * dz;
    if (distSq <= (double)(mStopDistance * mStopDistance))
    {
        mNavigation->stop();
        LookControl* targetLook = target->getLookControl();
        if (distSq <= (double)mStopDistance
            || (targetLook->getWantedX() == mMob->mX && targetLook->getWantedY() == mMob->mY
                && targetLook->getWantedZ() == mMob->mZ))
        {
            double followX = target->mX - mMob->mX;
            double followZ = target->mZ - mMob->mZ;
            mNavigation->moveTo(mMob->mX - followX, mMob->mY, mMob->mZ - followZ, mSpeedModifier);
        }
        return;
    }

    mNavigation->moveTo(target, mSpeedModifier);
}

bool FollowMobGoal::FollowPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    return entity && mMob->GetType() != entity->GetType();
}
