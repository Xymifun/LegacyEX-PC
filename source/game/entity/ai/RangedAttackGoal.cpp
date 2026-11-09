#include "game/entity/ai/RangedAttackGoal.hpp"

#include "game/core/Mth.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"

RangedAttackGoal::RangedAttackGoal(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackInterval,
                                   float attackRadius)
    : mTarget()
{
    _init(rangedAttackMob, mob, speedModifier, attackInterval, attackInterval, attackRadius);
}

RangedAttackGoal::RangedAttackGoal(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier,
                                   int attackIntervalMin, int attackIntervalMax, float attackRadius)
    : mTarget()
{
    _init(rangedAttackMob, mob, speedModifier, attackIntervalMin, attackIntervalMax, attackRadius);
}

void RangedAttackGoal::_init(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackIntervalMin,
                             int attackIntervalMax, float attackRadius)
{
    mRangedAttackMob = rangedAttackMob;
    mMob = mob;
    mSpeedModifier = speedModifier;
    mAttackIntervalMin = attackIntervalMin;
    mAttackIntervalMax = attackIntervalMax;
    mAttackRadius = attackRadius;
    mAttackRadiusSqr = attackRadius * attackRadius;
    setRequiredControlFlags(3);
    mTarget = boost::weak_ptr<LivingEntity>();
    mSeeTime = 0;
    mAttackTime = -1;
}

bool RangedAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return false;
    mTarget = boost::weak_ptr<LivingEntity>(target);
    return true;
}

bool RangedAttackGoal::canContinueToUse()
{
    return canUse() || !mMob->getNavigation()->isDone();
}

void RangedAttackGoal::stop()
{
    mTarget = boost::weak_ptr<LivingEntity>();
    mSeeTime = 0;
    mAttackTime = -1;
}

void RangedAttackGoal::tick()
{
    if (!mTarget.lock())
        return;

    double distance = mMob->distanceToSqr(mTarget.lock()->mX, mTarget.lock()->mBoundingBox->mMinY, mTarget.lock()->mZ);
    bool canSee = mMob->getSensing()->canSee(mTarget.lock());
    if (canSee)
        ++mSeeTime;
    else
        mSeeTime = 0;

    if (distance > (double)mAttackRadiusSqr || mSeeTime < 20)
        mMob->getNavigation()->moveTo(mTarget.lock(), mSpeedModifier);
    else
        mMob->getNavigation()->stop();

    static_cast<LookControl*>(mMob->getLookControl())->setLookAt(mTarget.lock(), 30.0f, 30.0f);

    if (--mAttackTime == 0)
    {
        if (!canSee)
            return;
        float distanceFactor = Mth::sqrt(distance) / mAttackRadius;
        float attackPower = Mth::clamp(distanceFactor, 0.1f, 1.0f);
        mRangedAttackMob->performRangedAttack(mTarget.lock(), attackPower);
        mAttackTime = Mth::floor(distanceFactor * (float)(mAttackIntervalMax - mAttackIntervalMin)
                                 + (float)mAttackIntervalMin);
    }
    else if (mAttackTime < 0)
    {
        float distanceFactor = Mth::sqrt(distance) / mAttackRadius;
        mAttackTime = Mth::floor(distanceFactor * (float)(mAttackIntervalMax - mAttackIntervalMin)
                                 + (float)mAttackIntervalMin);
    }
}
