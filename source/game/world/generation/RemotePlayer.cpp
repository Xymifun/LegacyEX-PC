#include "game/world/generation/RemotePlayer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/GameType.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockPos.hpp"

RemotePlayer::RemotePlayer(Level* level, const std::wstring& name) : AbstractClientPlayer(level, name)
{
    fjDerivedCtorCalls();
    mField0x880 = 0.0f;
    mField0x878 = 0;
    mMaxUpStep = 1.0f;
    mNoPhysics = true;
    mBedOffsetY = 0.25f;
    sViewScale = 10.0;
}

bool RemotePlayer::hasPermission(EGameCommand)
{
    return false;
}

BlockPos RemotePlayer::getCommandSenderBlockPosition()
{
    return BlockPos((int)std::floor(mX + 0.5), (int)std::floor(mY + 0.5), (int)std::floor(mZ + 0.5));
}

bool RemotePlayer::hurt(const DamageSource*, float)
{
    return true;
}

void RemotePlayer::tick()
{
    mBedOffsetY = 0;
    AbstractClientPlayer::tick();

    mAnimationSpeedOld = mAnimationSpeed;
    double xd = mX - mXo;
    double zd = mZ - mZo;
    float speed = Mth::sqrt(xd * xd + zd * zd) * 4.0f;
    if (speed > 1.0f)
        speed = 1.0f;
    mAnimationSpeed += (speed - mAnimationSpeed) * 0.4f;
    mAnimationPosition += mAnimationSpeed;
}

void RemotePlayer::aiStep()
{
    if (mLerpSteps > 0)
    {
        double lerpX = mX + (mLerpX - mX) / mLerpSteps;
        double lerpY = mY + (mLerpY - mY) / mLerpSteps;
        double lerpZ = mZ + (mLerpZ - mZ) / mLerpSteps;
        double yRotDelta = mLerpYRot - mYRot;
        while (yRotDelta < -180)
            yRotDelta += 360;
        while (yRotDelta >= 180)
            yRotDelta -= 360;
        mYRot += (float)(yRotDelta / mLerpSteps);
        mXRot += (float)((mLerpXRot - mXRot) / mLerpSteps);
        mLerpSteps--;
        setPos(lerpX, lerpY, lerpZ);
        setRot(mYRot, mXRot);
        mYBodyRot = mYRot;
    }

    mPrevCameraYaw = mCameraYaw;
    updateSwingTime();
    float horizontalSpeed = Mth::sqrt(mXd * mXd + mZd * mZd);
    float verticalAngle = static_cast<float>(std::atan(-mYd * 0.2f)) * 15.0f;
    if (horizontalSpeed > 0.1f)
        horizontalSpeed = 0.1f;
    if (!mOnGround || getHealth() <= 0.0f)
        horizontalSpeed = 0.0f;
    if (mOnGround || getHealth() <= 0.0f)
        verticalAngle = 0.0f;
    mCameraYaw += (horizontalSpeed - mCameraYaw) * 0.4f;
    mBob += (verticalAngle - mBob) * 0.8f;
    calculateFlapping();
}

void RemotePlayer::animateRespawn()
{
}

void RemotePlayer::SetGameType(const GameType* gameType)
{
    mGameType = gameType;
    Player::UpdateSize();
    mGameType->updatePlayerAbilities(&mAbilities);
}

const GameType* RemotePlayer::GetGameType()
{
    return mGameType;
}

unsigned long long RemotePlayer::GetType()
{
    return EntityType::REMOTE_PLAYER;
}
