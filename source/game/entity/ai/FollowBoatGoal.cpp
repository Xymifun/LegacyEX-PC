#include "game/entity/ai/FollowBoatGoal.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/Boat.hpp"
#include "game/core/Dolphin.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

FollowBoatGoal::FollowBoatGoal(Dolphin* dolphin)
    : mSearchDistance(4),
      mMaxDistance(12),
      mCooldown(0),
      mTarget(),
      mState(0),
      mDolphin(dolphin)
{
}

bool FollowBoatGoal::canUse()
{
    std::vector<boost::shared_ptr<Entity> >* boats = mDolphin->mLevel->getEntitiesOfClass(
        typeid(Boat), mDolphin->mBoundingBox->grow(5.0));
    bool boatMoving = false;
    for (unsigned int i = 0; i < boats->size(); ++i)
    {
        boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(boats->at(i));
        if (boat->getControllingPassenger() != NULL
            && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, boat->getControllingPassenger()->GetType()))
        {
            boost::shared_ptr<LivingEntity> passenger = boost::static_pointer_cast<LivingEntity>(
                boat->getControllingPassenger());
            if (Mth::abs(passenger->mXxa) > 0.0f || Mth::abs(passenger->mZza) > 0.0f)
            {
                boatMoving = true;
                break;
            }
        }
    }
    delete boats;
    return mTarget.expired() && boatMoving;
}

bool FollowBoatGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mTarget.lock();
    return target != NULL && target->isPassenger() && (Mth::abs(target->mXxa) > 0.0f || Mth::abs(target->mZza) > 0.0f);
}

bool FollowBoatGoal::canInterrupt()
{
    return true;
}

void FollowBoatGoal::start()
{
    std::vector<boost::shared_ptr<Entity> >* boats = mDolphin->mLevel->getEntitiesOfClass(
        typeid(Boat), mDolphin->mBoundingBox->grow(5.0));
    for (unsigned int i = 0; i < boats->size(); ++i)
    {
        boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(boats->at(i));
        if (boat->getControllingPassenger() != NULL
            && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, boat->getControllingPassenger()->GetType()))
        {
            mTarget = boost::static_pointer_cast<LivingEntity>(boat->getControllingPassenger());
            break;
        }
    }
    mCooldown = 0;
    mState = 0;
    delete boats;
}

void FollowBoatGoal::stop()
{
    mTarget.reset();
}

void FollowBoatGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mTarget.lock();
    if (target == NULL)
    {
        return;
    }

    bool targetMoving = Mth::abs(target->mXxa) > 0.0f || Mth::abs(target->mZza) > 0.0f;
    float speed = 0.15f;
    if (mState == 1)
    {
        speed = targetMoving ? 0.2f : 0.0f;
    }
    mDolphin->moveRelative(mDolphin->mXxa, mDolphin->mYya, mDolphin->mZza, speed);
    mDolphin->move(eMoverType_Self, mDolphin->mXd, mDolphin->mYd, mDolphin->mZd, false);
    mDolphin->mXd *= 0.9f;
    mDolphin->mYd *= 0.9f;
    mDolphin->mZd *= 0.9f;

    if (--mCooldown > 0)
    {
        return;
    }
    mCooldown = 10;

    switch (mState)
    {
    case 0:
    {
        BlockPos behind = BlockPos(target).relative(target->getDirection()->getOpposite());
        behind = behind.offset(0, -1, 0);
        mDolphin->getNavigation()->moveTo(behind.x, behind.y, behind.z, 1.0);
        if (mDolphin->distanceTo(target) < static_cast<float>(mSearchDistance))
        {
            mCooldown = 0;
            mState = 1;
        }
        break;
    }
    case 1:
    {
        const Direction* motion = target->getMotionDirection();
        BlockPos ahead = BlockPos(target).relative(motion, 10);
        mDolphin->getNavigation()->moveTo(ahead.x, ahead.y - 1, ahead.z, 1.0);
        if (mDolphin->distanceTo(target) > static_cast<float>(mMaxDistance))
        {
            mCooldown = 0;
            mState = 0;
        }
        break;
    }
    }
}

std::wstring FollowBoatGoal::getName()
{
    return std::wstring(L"Follow Boat");
}
