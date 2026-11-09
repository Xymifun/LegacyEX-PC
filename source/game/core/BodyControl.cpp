#include "game/core/BodyControl.hpp"
#include "game/core/EntityType.hpp"

#include <math.h>
#include <vector>

#include <boost/weak_ptr.hpp>

#include "game/core/WitherBoss.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Mth.hpp"

BodyControl::BodyControl(LivingEntity* mob) : mMob(mob), mHeadStableTime(0), mLastStableYHeadRot(0.0f)
{
}

float BodyControl::clamp(float base, float value, float maxDifference)
{
    float difference = Mth::wrapDegrees(base - value);

    if (difference < -maxDifference)
    {
        difference = -maxDifference;
    }

    if (difference >= maxDifference)
    {
        difference = maxDifference;
    }

    return base - difference;
}

void BodyControl::clientTick()
{
    if (mMob->GetType() == EntityType::WITHER_BOSS && static_cast<WitherBoss*>(mMob)->IsChargeAttacking())
    {
        mMob->mYBodyRot = mMob->mYHeadRot;
        return;
    }

    double xDelta = mMob->mX - mMob->mXo;
    double zDelta = mMob->mZ - mMob->mZo;

    if (xDelta * xDelta + zDelta * zDelta > MoveControl::MIN_SPEED_SQR)
    {
        mMob->mYBodyRot = mMob->mYRot;
        if (mMob->GetType() != EntityType::WITHER_BOSS)
        {
            mMob->mYHeadRot = clamp(mMob->mYBodyRot, mMob->mYHeadRot, 75.0f);
        }
        mLastStableYHeadRot = mMob->mYHeadRot;
        mHeadStableTime = 0;
    }
    else
    {
        std::vector<boost::weak_ptr<Entity> >* passengers = mMob->getPassengers();

        if (passengers->empty() || mMob->controllingPassengerIsA(EntityType::MOB))
        {
            float maxYRotFromBody = 75.0f;

            if (mMob->GetType() == EntityType::WITHER_BOSS)
            {
                maxYRotFromBody = 360.0f;
            }

            float yHeadRot = mMob->mYHeadRot;
            if (fabsf(yHeadRot - mLastStableYHeadRot) > 15.0f)
            {
                mLastStableYHeadRot = yHeadRot;
                mHeadStableTime = 0;
            }
            else
            {
                ++mHeadStableTime;

                if (mHeadStableTime > 10)
                {
                    float stableScale = 1.0f - (float)(mHeadStableTime - 10) / 10.0f;

                    if (stableScale < 0.0f)
                    {
                        stableScale = 0.0f;
                    }

                    maxYRotFromBody = maxYRotFromBody * stableScale;
                }
            }

            mMob->mYBodyRot = clamp(yHeadRot, mMob->mYBodyRot, maxYRotFromBody);

            if (mMob->GetType() == EntityType::WITHER_BOSS)
            {
                mMob->mYRot = mMob->mYBodyRot;
            }
        }
    }
}
