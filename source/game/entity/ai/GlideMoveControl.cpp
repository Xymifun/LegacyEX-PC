#include "game/entity/ai/GlideMoveControl.hpp"

#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Vec3.hpp"

GlideMoveControl::GlideMoveControl(Mob* mob, float startSpeed, float speedWhenTurning)
    : MoveControl(mob),
      mStartSpeed(startSpeed),
      mSpeedWhenTurning(speedWhenTurning),
      mSpeed(startSpeed)
{
}

GlideMoveControl::~GlideMoveControl()
{
}

void GlideMoveControl::tick()
{
    if (mOperation != MOVE_TO)
    {
        return;
    }

    if (mMob->mHorizontalCollision)
    {
        mMob->mYRot += 180.0f;
        mSpeed = mStartSpeed;
    }

    double dz = mWantedZ - mMob->mZ;
    double dx = mWantedX - mMob->mX;
    double dy = mWantedY - mMob->mY;
    double horizontalDistance = Mth::sqrt(dx * dx + dz * dz);

    double horizontalScale = 1.0;
    if (horizontalDistance != 0.0)
    {
        horizontalScale = 1.0 - fabs(dy * 0.7) / horizontalDistance;
    }

    dx *= horizontalScale;
    dz *= horizontalScale;
    double horizontalSquared = dx * dx + dz * dz;
    double horizontalLength = Mth::sqrt(horizontalSquared);
    double distance = Mth::sqrt(dy * dy + horizontalSquared);
    float oldYRot = mMob->mYRot;
    float wantedAngle = (float)atan2(dz, dx);
    float currentYRot = Mth::wrapDegrees(90.0f + mMob->mYRot);
    float wantedYRot = Mth::wrapDegrees(wantedAngle * 57.295776f);
    float yRot = Mth::approachDegrees(currentYRot, wantedYRot, 4.0f) - 90.0f;
    mMob->mYBodyRot = yRot;
    mMob->mYRot = yRot;

    if (Mth::degreesDifferenceAbs(oldYRot, mMob->mYRot) < 3.0f)
    {
        float movementSpeed = (float)mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
        mSpeed = Mth::approach(mSpeed, (float)(movementSpeed * mSpeedModifier), 0.005f * (1.8f / mSpeed));
    }
    else
    {
        mSpeed = Mth::approach(mSpeed, (float)(mSpeedWhenTurning * mSpeedModifier), 0.025f);
    }

    float wantedXRot = -((float)atan2(-dy, horizontalLength) * 57.295776f);
    static_cast<LookControl*>(mMob->getLookControl())
        ->setLookRotation(Vec3::newTemp((double)wantedXRot, (double)mMob->mYRot, 0.0), 60.0f, 20.0f);

    float yawRadians = (90.0f + mMob->mYRot) * Mth::DEG_TO_RAD;
    float pitchRadians = wantedXRot * Mth::DEG_TO_RAD;
    float xSpeed = mSpeed * cosf(yawRadians) * fabsf((float)(dx / distance));
    float zSpeed = mSpeed * sinf(yawRadians) * fabsf((float)(dz / distance));
    float ySpeed = mSpeed * sinf(pitchRadians) * fabsf((float)(dy / distance));
    mMob->mXd += ((double)xSpeed - mMob->mXd) * 0.2;
    mMob->mZd += ((double)zSpeed - mMob->mZd) * 0.2;
    mMob->mYd += ((double)ySpeed - mMob->mYd) * 0.2;
    mMob->mFallDistance = 0.0f;
}
