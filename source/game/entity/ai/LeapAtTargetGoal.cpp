#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/Difficulty.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

LeapAtTargetGoal::LeapAtTargetGoal(Mob* mob, float yd)
{
    mTarget = boost::weak_ptr<LivingEntity>();
    mMob = mob;
    mYd = yd;
    setRequiredControlFlags(5);
}

bool LeapAtTargetGoal::canUse()
{
    mTarget = boost::weak_ptr<LivingEntity>(mMob->getTarget());
    if (!mTarget.lock())
    {
        return false;
    }
    double distance = mMob->distanceToSqr(mTarget.lock());
    if (distance < 4.0 || distance > 16.0 || !mMob->mOnGround || mMob->getRandom()->nextInt(5) != 0)
    {
        return false;
    }
    return true;
}

bool LeapAtTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mTarget.lock();
    if (target)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType()))
        {
            return target->mLevel->getDifficulty() != Difficulty::PEACEFUL && !mMob->mOnGround;
        }
        return !mMob->mOnGround;
    }
    return false;
}

void LeapAtTargetGoal::start()
{
    double dx = mTarget.lock()->mX - mMob->mX;
    double dz = mTarget.lock()->mZ - mMob->mZ;
    double distanceSqr = dx * dx + dz * dz;
    double distance = std::sqrt(distanceSqr);
    if (distance >= 0.0001)
    {
        mMob->mXd += dx / distance * 0.5 * 0.800000011920929 + mMob->mXd * 0.2f;
        mMob->mZd += dz / distance * 0.5 * 0.800000011920929 + mMob->mZd * 0.2f;
    }
    mMob->mYd = static_cast<double>(mYd);
}
