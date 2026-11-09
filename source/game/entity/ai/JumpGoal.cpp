#include "game/entity/ai/JumpGoal.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/Dolphin.hpp"
#include "game/core/Mth.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/Level.hpp"

JumpGoal::JumpGoal(Dolphin* mob, int interval) : Goal(), mInterval(interval), mMob(mob), mCanJump(false)
{
}

JumpGoal::~JumpGoal()
{
}

bool JumpGoal::isObstacle()
{
    Level* level = mMob->mLevel;
    int stepX = mMob->getMotionDirection()->getStepX();
    int stepZ = mMob->getMotionDirection()->getStepZ();
    return level->getBlockState(BlockPos(mMob).offset(stepX * 2, 0, stepZ * 2))->isSolidBlockingCube()
           && level->getBlockState(BlockPos(mMob).offset(stepX, 0, stepZ))->getMaterial() == Material::WATER
           && level->getBlockState(BlockPos(mMob).offset(stepX, 1, stepZ))->getMaterial() == Material::AIR
           && level->getBlockState(BlockPos(mMob).offset(stepX, 2, stepZ))->getMaterial() == Material::AIR
           && level->getBlockState(BlockPos(mMob).offset(stepX * 4, 0, stepZ * 4))->getMaterial() == Material::WATER;
}

bool JumpGoal::canUse()
{
    if (mMob->getRandom()->nextInt(mInterval) != 0)
    {
        return false;
    }
    const Direction* direction = mMob->getMotionDirection();
    BlockPos position(mMob);
    bool clear = true;
    for (int distance = 1; distance <= 5; ++distance)
    {
        if (mMob->mLevel->getBlockState(position.relative(direction, distance))->getMaterial() != Material::WATER)
        {
            clear = false;
        }
    }
    return (isObstacle() || clear)
           && mMob->mLevel->getBlockState(BlockPos(mMob).above(2))->getMaterial() == Material::AIR;
}

bool JumpGoal::canContinueToUse()
{
    int finished = 1;
    if (!(mMob->mYd * mMob->mYd < 0.03 && mMob->mXRot != 0.0f && std::fabs(mMob->mXRot) < 10.0f && mMob->isInWater()))
    {
        if (!mMob->mOnGround)
        {
            finished = 0;
        }
    }
    return !finished;
}

bool JumpGoal::canInterrupt()
{
    return false;
}

void JumpGoal::start()
{
    const Direction* direction = mMob->getMotionDirection();
    mMob->mXd += static_cast<double>(direction->getStepX()) * 0.6;
    mMob->mYd += 0.7;
    mMob->mZd += static_cast<double>(direction->getStepZ()) * 0.6;
    mMob->getNavigation()->stop();
}

void JumpGoal::stop()
{
    mMob->mXRot = 0.0f;
}

float JumpGoal::rotlerp(float from, float to, float amount)
{
    float difference = to - from;
    while (difference < -180.0f)
    {
        difference += 360.0f;
    }
    while (difference >= 180.0f)
    {
        difference -= 360.0f;
    }
    return from + amount * difference;
}

void JumpGoal::tick()
{
    if (!mCanJump && !mMob->isUnderLiquid(Material::WATER))
    {
        mCanJump = true;
        mMob->playSound(SoundEvents::DOLPHIN_JUMP, 1.0f, 1.0f);
    }
    double verticalSpeed = mMob->mYd * mMob->mYd;
    if (verticalSpeed < 0.03 && mMob->mXRot != 0.0f)
    {
        mMob->mXRot = rotlerp(mMob->mXRot, 0.0f, 0.2f);
        return;
    }
    double speed = Mth::sqrt(mMob->mXd * mMob->mXd + verticalSpeed + mMob->mZd * mMob->mZd);
    double horizontalSpeed = Mth::sqrt(mMob->mXd * mMob->mXd + mMob->mZd * mMob->mZd);
    double fallSpeed = -mMob->mYd;
    double direction = static_cast<double>((0.0 < fallSpeed) - (fallSpeed < 0.0));
    mMob->mXRot = static_cast<float>(direction * std::acos(Mth::clamp(horizontalSpeed / speed, -1.0, 1.0))
                                     * Mth::RAD_TO_DEG);
}

std::wstring JumpGoal::getName()
{
    return std::wstring(L"Jump Goal");
}
