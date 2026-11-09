#include "game/entity/ai/DolphinLookControl.hpp"

#include <math.h>

#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"

DolphinLookControl::DolphinLookControl(Mob* mob, int maxYRotFromCenter)
    : LookControl(mob),
      mMaxYRotFromCenter(maxYRotFromCenter)
{
}

DolphinLookControl::~DolphinLookControl()
{
}

void DolphinLookControl::tick()
{
    if (mHasWanted)
    {
        mHasWanted = false;
        double x = mWantedX - mMob->mX;
        double y = mWantedY - (mMob->mY + mMob->getEyeHeight());
        double z = mWantedZ - mMob->mZ;
        double horizontalDistance = Mth::sqrt(x * x + z * z);
        float wantedYRot = (float)::atan2(z, x) * Mth::RAD_TO_DEG + -70.0f;
        float wantedXRot = 10.0f - (float)::atan2(y, horizontalDistance) * Mth::RAD_TO_DEG;
        mMob->mXRot = rotlerp(mMob->mXRot, wantedXRot, mXMax);
        mMob->mYHeadRot = rotlerp(mMob->mYHeadRot, wantedYRot, mYMax);
    }
    else
    {
        if (mMob->getNavigation()->isDone())
        {
            mMob->mXRot = rotlerp(mMob->mXRot, 0.0f, 5.0f);
        }
        mMob->mYHeadRot = rotlerp(mMob->mYHeadRot, mMob->mYBodyRot, mYMax);
    }

    float yRotDifference = Mth::wrapDegrees(mMob->mYHeadRot - mMob->mYBodyRot);
    if (yRotDifference < (float)(-mMaxYRotFromCenter))
    {
        mMob->mYBodyRot -= 4.0f;
    }
    else if (yRotDifference > (float)mMaxYRotFromCenter)
    {
        mMob->mYBodyRot += 4.0f;
    }
}
