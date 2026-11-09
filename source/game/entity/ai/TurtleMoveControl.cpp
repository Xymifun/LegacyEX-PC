#include "game/entity/ai/TurtleMoveControl.hpp"

#include <algorithm>
#include <math.h>

#include "game/core/AttributeInstance.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"

TurtleMoveControl::TurtleMoveControl(Turtle* turtle) : MoveControl(turtle), mTurtle(turtle)
{
}

TurtleMoveControl::~TurtleMoveControl()
{
}

void TurtleMoveControl::updateSpeed()
{
    if (mTurtle->isInWater())
    {
        mTurtle->mYd += 0.005;
        if (mTurtle->distanceToSqr(mTurtle->getHomePos()) > 256.0)
        {
            mTurtle->setSpeed(std::max(mTurtle->getSpeed() * 0.5f, 0.08f));
        }
    }
    else if (mTurtle->mOnGround)
    {
        mTurtle->setSpeed(std::max(mTurtle->getSpeed() * 0.5f, 0.06f));
    }
}

void TurtleMoveControl::tick()
{
    if (mTurtle->isLayingEgg())
    {
        mTurtle->setSpeed(0.0f);
        return;
    }

    updateSpeed();

    if (mOperation == JUMPING)
    {
        mMob->setSpeed(
            (float)(mSpeedModifier * mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
        if (mMob->mOnGround)
        {
            mOperation = WAIT;
        }
    }
    else if (mOperation != MOVE_TO || mTurtle->getNavigation()->isDone())
    {
        mTurtle->setSpeed(0.0f);
        return;
    }

    double x = mWantedX - mTurtle->mX;
    double y = mWantedY - mTurtle->mY;
    double z = mWantedZ - mTurtle->mZ;
    double distance = Mth::sqrt(x * x + y * y + z * z);
    y = y / distance;
    float wantedYRot = (float)atan2(z, x) * 57.295776f - 90.0f;
    float yRot = rotlerp(mTurtle->mYRot, wantedYRot, 30.0f);
    mTurtle->mYBodyRot = mTurtle->mYRot = yRot;

    float wantedSpeed = (float)(mSpeedModifier
                                * mTurtle->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());
    mTurtle->setSpeed(mTurtle->getSpeed() + (wantedSpeed - mTurtle->getSpeed()) * 0.125f);

    mTurtle->mYd += (double)mTurtle->getSpeed() * y * 0.1;

    if (y > (double)mMob->mMaxUpStep && x * x + z * z < (double)std::max(1.0f, mMob->mBbWidth))
    {
        static_cast<JumpControl*>(mMob->getJumpControl())->jump();
        mOperation = JUMPING;
    }
}
