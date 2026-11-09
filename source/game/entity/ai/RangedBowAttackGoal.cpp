#include "game/entity/ai/RangedBowAttackGoal.hpp"

#include "game/core/Sensing.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"

RangedBowAttackGoal::RangedBowAttackGoal(Monster* mob, double speedModifier, int attackInterval, float attackRadius)
    : Goal(),
      mAttackTime(-1),
      mSeeTime(0),
      mStrafingClockwise(false),
      mStrafingBackwards(false),
      mStrafingTime(-1)
{
    mMob = mob;
    mSpeedModifier = speedModifier;
    mAttackInterval = attackInterval;
    mAttackRadiusSqr = attackRadius * attackRadius;
    setRequiredControlFlags(3);
}

RangedBowAttackGoal::~RangedBowAttackGoal()
{
}

void RangedBowAttackGoal::setMinAttackInterval(int attackInterval)
{
    mAttackInterval = attackInterval;
}

bool RangedBowAttackGoal::isHoldingBow()
{
    return !mMob->getMainHandItem()->isEmpty() && mMob->getMainHandItem()->getItem() == Items::bow;
}

bool RangedBowAttackGoal::canUse()
{
    if (!mMob->getTarget())
        return false;
    return isHoldingBow();
}

bool RangedBowAttackGoal::canContinueToUse()
{
    return (canUse() || !mMob->getNavigation()->isDone()) && isHoldingBow();
}

void RangedBowAttackGoal::start()
{
    Goal::start();
    dynamic_cast<RangedAttackMob*>(mMob)->setAggressive(true);
}

void RangedBowAttackGoal::stop()
{
    Goal::stop();
    dynamic_cast<RangedAttackMob*>(mMob)->setAggressive(false);
    mSeeTime = 0;
    mAttackTime = -1;
    mMob->stopUsingItem();
}

void RangedBowAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return;

    double distance = mMob->distanceToSqr(target->mX, target->getBoundingBox()->mMinY, target->mZ);
    bool canSee = mMob->getSensing()->canSee(target);
    if (canSee != (mSeeTime > 0))
        mSeeTime = 0;

    if (canSee)
        ++mSeeTime;
    else
        --mSeeTime;

    if (distance > (double)mAttackRadiusSqr || mSeeTime < 20)
    {
        mMob->getNavigation()->moveTo(target, mSpeedModifier);
        mStrafingTime = -1;
    }
    else
    {
        mMob->getNavigation()->stop();
        ++mStrafingTime;
    }

    if (mStrafingTime >= 20)
    {
        if ((double)mMob->getRandom()->nextFloat() < 0.3)
            mStrafingClockwise = !mStrafingClockwise;
        if ((double)mMob->getRandom()->nextFloat() < 0.3)
            mStrafingBackwards = !mStrafingBackwards;
        mStrafingTime = 0;
    }

    if (mStrafingTime > -1)
    {
        if (distance > (double)(mAttackRadiusSqr * 0.75f))
            mStrafingBackwards = false;
        else if (distance < (double)(mAttackRadiusSqr * 0.25f))
            mStrafingBackwards = true;

        mMob->getMoveControl()->strafe(mStrafingBackwards ? -0.5f : 0.5f, mStrafingClockwise ? 0.5f : -0.5f);
        mMob->lookAt(target, 30.0f, 30.0f);
    }
    else
    {
        mMob->getLookControl()->setLookAt(target, 30.0f, 30.0f);
    }

    if (mMob->isUsingItem())
    {
        if (!canSee)
        {
            if (mSeeTime < -60)
                mMob->stopUsingItem();
        }
        else
        {
            int ticksUsingItem = mMob->getTicksUsingItem();
            if (ticksUsingItem >= BowItem::GetMaxBowDuration())
            {
                mMob->stopUsingItem();
                dynamic_cast<RangedAttackMob*>(mMob)->performRangedAttack(target,
                                                                          BowItem::getPowerForTime(ticksUsingItem));
                mAttackTime = mAttackInterval;
            }
        }
    }
    else if (--mAttackTime <= 0 && mSeeTime >= -60)
    {
        mMob->startUsingItem(InteractionHand::eInteractionHand_MainHand);
    }
}
