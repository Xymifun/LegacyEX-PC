#include "game/entity/ai/SwoopAttackGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"

SwoopAttackGoal::SwoopAttackGoal(Mob* mob, float speedModifier, Goal::FloatRange attackDistance)
    : Goal(),
      mMob(mob),
      mSpeedModifier(speedModifier),
      mAttackDistance(attackDistance),
      mAttackInterval(0),
      mSwoopTicks(0),
      mSwooping(false)
{
    setRequiredControlFlags(3);
}

SwoopAttackGoal::~SwoopAttackGoal()
{
}

bool SwoopAttackGoal::canUse()
{
    if (mMob->getMoveControl() == NULL)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
    {
        return false;
    }

    if (mAttackInterval > 0)
    {
        --mAttackInterval;
        return false;
    }

    mAttackInterval =
        (int)(mAttackDistance.min + mMob->getRandom()->nextFloat() * (mAttackDistance.max - mAttackDistance.min)) * 20;
    return true;
}

void SwoopAttackGoal::start()
{
    mSwooping = false;
    mMob->playSound(SoundEvents::PHANTOM_SWOOP, 10.0f, 1.0f);
    mSwoopTicks = 0;
}

void SwoopAttackGoal::stop()
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->stop();
    }
    else
    {
        mMob->getMoveControl()->wait();
    }
}

bool SwoopAttackGoal::canContinueToUse()
{
    if (mMob->getTarget() == NULL || mSwooping || mMob->mHorizontalCollision || mMob->mHurtTime > 0)
    {
        return false;
    }
    return true;
}

void SwoopAttackGoal::tick()
{
    --mSwoopTicks;
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (target && mSwoopTicks < 1)
    {
        Vec3* targetPosition = target->getPos(0.0f);
        Vec3* wantedPosition = Vec3::newTemp(targetPosition->x, targetPosition->y + (double)target->mBbHeight,
                                             targetPosition->z);
        PathNavigation* navigation = mMob->getNavigation();
        if (navigation != NULL)
        {
            navigation->moveTo(wantedPosition->x, wantedPosition->y, wantedPosition->z, (double)mSpeedModifier);
        }
        else
        {
            mMob->getMoveControl()->setWantedPosition(wantedPosition->x, wantedPosition->y, wantedPosition->z,
                                                      (double)mSpeedModifier);
        }

        if (mMob->getBoundingBox()->grow(0.2)->intersects(target->getBoundingBox()))
        {
            mMob->doHurtTarget(target);
            mSwooping = true;
            mMob->playSound(SoundEvents::PHANTOM_BITE, 1.0f, 1.0f);
        }

        double distance = mMob->distanceToSqr(target->mX, target->getBoundingBox()->mMinY, target->mZ);
        if (distance > 1024.0)
        {
            mSwoopTicks = 5;
        }
        else if (distance > 256.0)
        {
            mSwoopTicks = 2;
        }
    }
}
