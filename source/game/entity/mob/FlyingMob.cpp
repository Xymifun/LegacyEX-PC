#include "game/entity/mob/FlyingMob.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

FlyingMob::FlyingMob(Level* level) : Mob(level)
{
}

void FlyingMob::causeFallDamage(float fallDistance, float damageMultiplier)
{
}

void FlyingMob::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
}

void FlyingMob::travel(float xxa, float yya, float zza)
{
    if (isInWater())
    {
        moveRelative(xxa, yya, zza, 0.02f);
        move(eMoverType_Self, mXd, mYd, mZd, false);
        mXd *= 0.800000011920929;
        mYd *= 0.800000011920929;
        mZd *= 0.800000011920929;
        mAnimationSpeedOld = mAnimationSpeed;
    }
    else if (isInLava())
    {
        moveRelative(xxa, yya, zza, 0.02f);
        move(eMoverType_Self, mXd, mYd, mZd, false);
        mXd *= 0.5;
        mYd *= 0.5;
        mZd *= 0.5;
        mAnimationSpeedOld = mAnimationSpeed;
    }
    else
    {
        float friction = 0.91f;

        if (mOnGround)
        {
            friction =
                mLevel->getBlockState(BlockPos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY) - 1, Mth::floor(mZ)))
                    ->getBlock()
                    ->mFriction
                * 0.91f;
        }

        float friction3 = 0.16277136f / (friction * friction * friction);

        if (mOnGround)
        {
            moveRelative(xxa, yya, zza, 0.1f * friction3);
        }
        else
        {
            moveRelative(xxa, yya, zza, 0.02f);
        }

        friction = 0.91f;

        if (mOnGround)
        {
            friction =
                mLevel->getBlockState(BlockPos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY) - 1, Mth::floor(mZ)))
                    ->getBlock()
                    ->mFriction
                * 0.91f;
        }

        move(eMoverType_Self, mXd, mYd, mZd, false);
        mXd *= friction;
        mYd *= friction;
        mZd *= friction;
        mAnimationSpeedOld = mAnimationSpeed;
    }

    double dx = mX - mXo;
    double dz = mZ - mZo;
    float target = (float)sqrt(dx * dx + dz * dz) * 4.0f;

    if (target > 1.0f)
    {
        target = 1.0f;
    }

    mAnimationSpeed = mAnimationSpeed + (target - mAnimationSpeed) * 0.4f;
    mAnimationPosition = mAnimationPosition + mAnimationSpeed;
}

bool FlyingMob::onLadder()
{
    return false;
}
