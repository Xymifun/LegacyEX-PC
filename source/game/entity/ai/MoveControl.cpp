#include "game/entity/ai/MoveControl.hpp"

#include <math.h>

#include <algorithm>

#include "game/core/AttributeInstance.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/NodeEvaluator.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/util/AABB.hpp"

float MoveControl::MIN_SPEED_SQR = 2.5000003E-7f;

MoveControl::MoveControl(Mob* mob)
    : mMob(mob),
      mWantedX(0.0),
      mWantedY(0.0),
      mWantedZ(0.0),
      mSpeedModifier(0.0),
      mStrafeForwards(0.0f),
      mStrafeRight(0.0f),
      mOperation(WAIT)
{
}

void MoveControl::copyFrom(MoveControl* other)
{
    mOperation = other->mOperation;
    mWantedX = other->mWantedX;
    mWantedY = other->mWantedY;
    mWantedZ = other->mWantedZ;
    mSpeedModifier = other->mSpeedModifier < 1.0 ? 1.0 : other->mSpeedModifier;
    mStrafeForwards = other->mStrafeForwards;
    mStrafeRight = other->mStrafeRight;
}

void MoveControl::wait()
{
    mOperation = WAIT;
}

bool MoveControl::hasWanted()
{
    return mOperation == MOVE_TO;
}

MoveControl::Operation MoveControl::getOperation() const
{
    return mOperation;
}

void MoveControl::setOperation(Operation operation)
{
    mOperation = operation;
}

double MoveControl::getSpeedModifier()
{
    return mSpeedModifier;
}

void MoveControl::setSpeedModifier(double speedModifier)
{
    mSpeedModifier = speedModifier;
}

void MoveControl::setWantedPosition(double x, double y, double z, double speedModifier)
{
    mWantedX = x;
    mWantedY = y;
    mWantedZ = z;
    mSpeedModifier = speedModifier;
    mOperation = MOVE_TO;
}

void MoveControl::strafe(float forwards, float right)
{
    mStrafeForwards = forwards;
    mOperation = STRAFE;
    mStrafeRight = right;
    mSpeedModifier = 0.25;
}

float MoveControl::rotlerp(float from, float to, float max)
{
    float delta = Mth::wrapDegrees(to - from);
    if (delta > max)
    {
        delta = max;
    }
    if (delta < -max)
    {
        delta = -max;
    }

    float result = from + delta;
    if (result >= 180.0f)
    {
        result -= 360.0f;
    }
    if (result < -180.0f)
    {
        result += 360.0f;
    }
    return result;
}

void MoveControl::tick()
{
    bool swimming = eTYPE_DERIVED_FROM(EntityType::WATER_ANIMAL, mMob->GetType())
                    && !eTYPE_DERIVED_FROM(EntityType::SQUID, mMob->GetType()) && mMob->isInWater();

    if (mOperation == STRAFE)
    {
        mOperation = WAIT;
        float movementSpeed = (float)mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
        float speed = (float)mSpeedModifier * movementSpeed;
        float friction = 0.91f;
        if (mMob->mOnGround)
        {
            BlockPos below(Mth::floor(mMob->mX), Mth::floor(mMob->mBoundingBox->mMinY - 1.0), Mth::floor(mMob->mZ));
            friction = mMob->mLevel->getBlockState(below)->getBlock()->mFriction * 0.91f;
        }

        float frictionFactor = 0.16277136f / (friction * friction * friction);
        float forwards = mStrafeForwards;
        float right = mStrafeRight;
        float acceleration = mMob->mOnGround ? speed * frictionFactor : mMob->mFlyingSpeed;
        float length = Mth::sqrt(right * right + forwards * forwards);
        if (length < 1.0f)
        {
            length = 1.0f;
        }
        length = acceleration / length;
        right *= length;
        forwards *= length;

        float sinYaw = Mth::sin(mMob->mYRot * 0.017453292f);
        float cosYaw = Mth::cos(mMob->mYRot * 0.017453292f);
        float xa = right * cosYaw - forwards * sinYaw;
        float za = forwards * cosYaw + right * sinYaw;

        PathNavigation* navigation = mMob->getNavigation();
        if (navigation != NULL)
        {
            NodeEvaluator* evaluator = navigation->getNodeEvaluator();
            BlockPos target(mMob->mX + xa + mMob->mXd, mMob->mY, mMob->mZ + za + mMob->mZd);
            if (evaluator != NULL)
            {
                bool blocked = evaluator->getBlockPathType(mMob->mLevel, target.x, target.y, target.z)
                               != BlockPathTypes::WALKABLE;
                bool sunlit = navigation->shouldAvoidSun() && mMob->mLevel->isDay() && mMob->mLevel->canSeeSky(target);

                if (blocked || sunlit)
                {
                    mMob->setSpeed(0.0f);
                    mMob->setZza(0.0f);
                    mMob->setXxa(0.0f);
                    if (!mMob->mHasImpulse)
                    {
                        mMob->mXd = 0.0;
                        mMob->mZd = 0.0;
                    }
                    return;
                }
            }
        }

        mMob->setSpeed(speed);
        mMob->setZza(mStrafeForwards);
        mMob->setXxa(mStrafeRight);
        mMob->setMoving(true);
    }
    else if (mOperation == MOVE_TO)
    {
        mOperation = WAIT;
        double dx = mWantedX - mMob->mX;
        double dz = mWantedZ - mMob->mZ;
        double dy = mWantedY - mMob->mY;
        if (dx * dx + dy * dy + dz * dz < MIN_SPEED_SQR)
        {
            mMob->setZza(0.0f);
            return;
        }

        float wantedYRot = 0.0f;
        double horizontalSqr = dx * dx + dz * dz;
        if (horizontalSqr > 0.1f)
        {
            wantedYRot = (float)(atan2(dz, dx) * 57.2957763671875) - 90.0f;
        }
        mMob->mYRot = rotlerp(mMob->mYRot, wantedYRot, 30.0f);
        mMob->setSpeed(
            (float)(mSpeedModifier * mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));

        if (!swimming && dy > 0.009f && horizontalSqr < std::max(1.0f, mMob->mBbWidth))
        {
            mMob->getJumpControl()->jump();
            mOperation = JUMPING;
        }
        mMob->setMoving(true);
    }
    else if (mOperation == JUMPING)
    {
        mMob->setSpeed(
            (float)(mSpeedModifier * mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
        if (mMob->mOnGround)
        {
            mOperation = WAIT;
        }
    }
    else
    {
        mMob->setZza(0.0f);
        mMob->setXxa(0.0f);
        mMob->setMoving(false);
    }

    if (swimming)
    {
        mMob->mYd += 0.005;
        double movementSpeed = mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
        mMob->setYya((float)(mWantedY + 0.5 - mMob->mY) * (float)(mSpeedModifier * movementSpeed));
    }
}

double MoveControl::getWantedX()
{
    return mWantedX;
}

double MoveControl::getWantedY()
{
    return mWantedY;
}

double MoveControl::getWantedZ()
{
    return mWantedZ;
}
