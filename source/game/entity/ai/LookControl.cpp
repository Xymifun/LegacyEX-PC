#include "game/entity/ai/LookControl.hpp"
#include "game/core/EntityType.hpp"

#include <math.h>

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"

LookControl::LookControl(Mob* mob)
    : mMob(mob),
      mYMax(0.0f),
      mXMax(0.0f),
      mHasWanted(false),
      mHasWantedRotation(false),
      mWantedRotation(NULL),
      mWantedX(0.0),
      mWantedY(0.0),
      mWantedZ(0.0)
{
}

void LookControl::setLookAt(boost::shared_ptr<Entity> entity, float yMaxRotationIncrease, float xMaxRotationIncrease)
{
    mWantedX = entity->mX;
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
    {
        mWantedY = boost::static_pointer_cast<LivingEntity>(entity)->getEyeHeight() + entity->mY;
    }
    else
    {
        mWantedY = (entity->mBoundingBox->mMinY + entity->mBoundingBox->mMaxY) * 0.5;
    }
    mWantedZ = entity->mZ;
    mYMax = yMaxRotationIncrease;
    mHasWanted = true;
    mXMax = xMaxRotationIncrease;
}

void LookControl::setLookAt(double x, double y, double z, float yMaxRotationIncrease, float xMaxRotationIncrease)
{
    mWantedX = x;
    mWantedY = y;
    mWantedZ = z;
    mYMax = yMaxRotationIncrease;
    mXMax = xMaxRotationIncrease;
    mHasWanted = true;
}

void LookControl::setLookRotation(const Vec3* rotation, float yMaxRotationIncrease, float xMaxRotationIncrease)
{
    if (mWantedRotation == NULL)
    {
        mWantedRotation = Vec3::newPermanent(rotation);
        mYMax = yMaxRotationIncrease;
        mHasWantedRotation = true;
        mXMax = xMaxRotationIncrease;
    }
}

float LookControl::rotlerp(float from, float to, float maxDelta)
{
    float delta = to - from;
    while (delta < -180.0f)
    {
        delta += 360.0f;
    }
    while (delta >= 180.0f)
    {
        delta -= 360.0f;
    }
    if (delta > maxDelta)
    {
        delta = maxDelta;
    }
    if (delta < -maxDelta)
    {
        delta = -maxDelta;
    }
    return from + delta;
}

void LookControl::tick()
{
    if (eTYPE_DERIVED_FROM(EntityType::WITHER_BOSS, mMob->GetType()))
    {
        return;
    }

    if (!mHasWanted)
    {
        if (!mHasWantedRotation)
        {
            mMob->mYHeadRot = rotlerp(mMob->mYHeadRot, mMob->mYBodyRot, 10.0f);
        }
        else
        {
            mHasWantedRotation = false;
            mMob->mXRot = Mth::clampRotateDegrees(mMob->mXRot, mWantedRotation->x, mXMax);
            mMob->mYHeadRot = Mth::clampRotateDegrees(mMob->mYHeadRot, mWantedRotation->y, mYMax);
            delete mWantedRotation;
            mWantedRotation = NULL;
        }
    }
    else
    {
        mHasWanted = false;
        double dx = mWantedX - mMob->mX;
        double dz = mWantedZ - mMob->mZ;
        double dy = mWantedY - (mMob->mY + mMob->getEyeHeight());
        double horizontal = sqrt(dx * dx + dz * dz);
        float wantedYRot = (float)(atan2(dz, dx) * Mth::RAD_TO_DEG) - 90.0f;
        float wantedXRot = (float)(-(atan2(dy, horizontal) * Mth::RAD_TO_DEG));
        mMob->mXRot = rotlerp(mMob->mXRot, wantedXRot, mXMax);
        mMob->mYHeadRot = rotlerp(mMob->mYHeadRot, wantedYRot, mYMax);
    }

    float delta = Mth::wrapDegrees(mMob->mYHeadRot - mMob->mYBodyRot);
    if (!mMob->getNavigation()->isDone())
    {
        if (delta < -75.0f)
        {
            mMob->mYHeadRot = mMob->mYBodyRot - 75.0f;
        }
        if (delta > 75.0f)
        {
            mMob->mYHeadRot = mMob->mYBodyRot + 75.0f;
        }
    }
}

bool LookControl::isHasWanted()
{
    return mHasWanted;
}

double LookControl::getWantedX()
{
    return mWantedX;
}

double LookControl::getWantedY()
{
    return mWantedY;
}

double LookControl::getWantedZ()
{
    return mWantedZ;
}
