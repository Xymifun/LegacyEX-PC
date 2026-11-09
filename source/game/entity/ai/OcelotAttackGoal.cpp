#include "game/entity/ai/OcelotAttackGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"

OcelotAttackGoal::OcelotAttackGoal(Mob* mob)
{
    mTarget = boost::weak_ptr<LivingEntity>();
    mMob = mob;
    mFlag = false;
    mAttackTime = 0;
    mValue = 0.0f;
    mLevel = mob->mLevel;
    setRequiredControlFlags(3);
}

bool OcelotAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return false;

    mTarget = boost::weak_ptr<LivingEntity>(target);
    return true;
}

bool OcelotAttackGoal::canContinueToUse()
{
    if (!mTarget.lock() || !mTarget.lock()->isAlive())
        return false;

    if (mMob->distanceToSqr(mTarget.lock()) > 225.0)
        return false;

    return !mMob->getNavigation()->isDone() || canUse();
}

void OcelotAttackGoal::stop()
{
    mTarget = boost::weak_ptr<Mob>();
    mMob->getNavigation()->stop();
}

void OcelotAttackGoal::tick()
{
    mMob->getLookControl()->setLookAt(mTarget.lock(), 30.0f, 30.0f);

    float attackReach = mMob->mBbWidth * 2.0f;
    double attackReachSqr = attackReach * attackReach;
    double distance = mMob->distanceToSqr(mTarget.lock()->mX, mTarget.lock()->mBoundingBox->mMinY, mTarget.lock()->mZ);

    double speed = 0.8;
    if (distance > attackReachSqr && distance < 16.0)
        speed = 1.33;
    else if (distance < 225.0)
        speed = 0.6;

    mMob->getNavigation()->moveTo(mTarget.lock(), speed);

    mAttackTime = mAttackTime - 1;
    if (mAttackTime < 0)
        mAttackTime = 0;

    if (distance <= attackReachSqr && mAttackTime <= 0)
    {
        mAttackTime = 20;
        mMob->doHurtTarget(mTarget.lock());
    }
}

void OcelotAttackGoal::setLevel(Level* level)
{
    mLevel = level;
}
