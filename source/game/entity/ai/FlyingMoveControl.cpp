#include "game/entity/ai/FlyingMoveControl.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

FlyingMoveControl::FlyingMoveControl(Mob* mob) : MoveControl(mob)
{
}

FlyingMoveControl::~FlyingMoveControl()
{
}

void FlyingMoveControl::tick()
{
    if (mOperation == MOVE_TO)
    {
        mOperation = WAIT;
        mMob->setNoGravity(true);

        double xDistance = mWantedX - mMob->mX;
        double zDistance = mWantedZ - mMob->mZ;
        double yDistance = mWantedY - mMob->mY;

        if (yDistance * yDistance + xDistance * xDistance + zDistance * zDistance < MIN_SPEED_SQR)
        {
            mMob->setYya(0.0f);
            mMob->setZza(0.0f);
            return;
        }

        float yRot = (float)(Mth::atan2((int)zDistance, (int)xDistance) * 57.2957763671875f) - 90.0f;
        float newYRot = rotlerp(mMob->mYRot, yRot, 10.0f);
        mMob->mYRot = newYRot;

        float speed;
        if (mMob->mOnGround)
        {
            speed = (float)(mSpeedModifier * mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());
        }
        else
        {
            speed = (float)(mSpeedModifier * mMob->getAttribute(SharedMonsterAttributes::FLYING_SPEED)->getValue());
        }
        mMob->setSpeed(speed);

        float horizontalDistance = Mth::sqrt(xDistance * xDistance + zDistance * zDistance);
        float xRot = -(Mth::atan2((int)yDistance, (int)horizontalDistance) * 57.2957763671875f);
        float newXRot = rotlerp(mMob->mXRot, xRot, 10.0f);
        mMob->mXRot = newXRot;
        mMob->setYya(yDistance > 0.0 ? speed : -speed);
    }
    else
    {
        mMob->setNoGravity(false);
        mMob->setYya(0.0f);
        mMob->setZza(0.0f);
    }
}
