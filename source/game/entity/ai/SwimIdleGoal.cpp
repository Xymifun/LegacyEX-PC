#include "game/entity/ai/SwimIdleGoal.hpp"
#include "game/core/EntityType.hpp"

#include <cstdlib>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/WaterAnimal.hpp"

SwimIdleGoal::SwimIdleGoal(Mob* mob) : Goal(), mMob(mob)
{
    mStartPos = NULL;
    mTicks = 0;
    setRequiredControlFlags(1);
}

SwimIdleGoal::~SwimIdleGoal()
{
}

bool SwimIdleGoal::canUse()
{
    if (!mMob->followsLeader() && !mMob->getNavigation()->isRiverFollowing() && mMob->getNavigation()->isDone())
    {
        bool result = false;
        if (mMob->isInWater() && mMob->getRandom()->nextInt(10) == 0)
        {
            result = true;
        }
        return result;
    }

    return false;
}

bool SwimIdleGoal::canContinueToUse()
{
    return mMob->isInWater() && mTicks < 100;
}

void SwimIdleGoal::start()
{
    mTicks = 0;
    mStartPos = mMob->getPos(0.0f);
    Goal::start();
}

void SwimIdleGoal::tick()
{
    ++mTicks;
    Goal::tick();

    mMob->mXd = 0.0;
    mMob->mYd = 0.0;
    mMob->mZd = 0.0;

    bool isWaterAnimal = eTYPE_DERIVED_FROM(EntityType::WATER_ANIMAL, mMob->GetType());
    if (!isWaterAnimal)
    {
        isWaterAnimal = eTYPE_DERIVED_FROM(EntityType::AGABLE_WATER_ANIMAL, mMob->GetType());
    }
    if (isWaterAnimal)
    {
        boost::shared_ptr<WaterAnimal> waterAnimal = boost::static_pointer_cast<WaterAnimal>(mMob->shared_from_this());
        if (waterAnimal->mSwimAmplitude > 0.0f)
        {
            const int id = mMob->getId();
            const float phase = static_cast<float>(std::abs(id) % 100);
            const float swim = Mth::sin((static_cast<float>(mMob->mTickCount) + phase) * waterAnimal->mSwimFrequency)
                               * waterAnimal->mSwimAmplitude;
            const float cosYaw = Mth::cos(mMob->mYRot * Mth::DEG_TO_RAD);
            const float sinYaw = Mth::sin(mMob->mYRot * Mth::DEG_TO_RAD);

            mMob->mXd += static_cast<double>(swim * cosYaw);
            mMob->mZd += static_cast<double>(swim * sinYaw);

            const int bobId = mMob->getId();
            const float bobPhase = static_cast<float>(std::abs(bobId) % 100);
            const float bob = Mth::sin((static_cast<float>(mMob->mTickCount) + bobPhase) * 0.75f);
            mMob->mYd += static_cast<double>(bob * 0.05f * (sinYaw + cosYaw) * 0.5f);
        }
    }
}
