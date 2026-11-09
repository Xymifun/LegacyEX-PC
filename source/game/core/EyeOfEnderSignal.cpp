#include "game/core/EyeOfEnderSignal.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Mth.hpp"

#include <cmath>

#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

EyeOfEnderSignal::EyeOfEnderSignal(Level* level, double x, double y, double z) : Entity(level, true)
{
    _init();
    mLife = 0;
    setSize(0.25f, 0.25f);
    setPos(x, y, z);
}

EyeOfEnderSignal::EyeOfEnderSignal(Level* level) : Entity(level, true)
{
    _init();
    setSize(0.25f, 0.25f);
}

void EyeOfEnderSignal::_init()
{
    fjDerivedCtorCalls();
    mUnknown = 0;
    mTargetZ = 0.0;
    mTargetY = 0.0;
    mTargetX = 0.0;
    mLife = 0;
    mDropItem = false;
}

void EyeOfEnderSignal::signalTo(const BlockPos& pos)
{
    double x = pos.x;
    int y = pos.y;
    double z = pos.z;
    double dx = x - mX;
    double dz = z - mZ;
    double distance = std::sqrt(dx * dx + dz * dz);
    if (distance > 12.0)
    {
        mTargetX = mX + dx / distance * 12.0;
        mTargetZ = mZ + dz / distance * 12.0;
        mTargetY = mY + 8.0;
    }
    else
    {
        mTargetX = x;
        mTargetY = y;
        mTargetZ = z;
    }
    mLife = 0;
    mDropItem = mRandom->nextInt(5) > 0;
}

void EyeOfEnderSignal::defineSynchedData()
{
}

bool EyeOfEnderSignal::shouldRenderAtSqrDistance(double distanceSqr)
{
    double distance = mBoundingBox->getSize() * 4.0;
    if (distance != distance)
    {
        distance = 4.0;
    }
    distance *= 64.0;
    return distanceSqr < distance * distance;
}

void EyeOfEnderSignal::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRotO == 0.0f && mYRotO == 0.0f)
    {
        double horizontal = std::sqrt(xd * xd + zd * zd);
        mYRotO = mYRot = static_cast<float>(std::atan2(xd, zd) * Mth::RAD_TO_DEG);
        mXRotO = mXRot = static_cast<float>(std::atan2(yd, horizontal) * Mth::RAD_TO_DEG);
    }
}

void EyeOfEnderSignal::tick()
{
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    Entity::tick();

    mX += mXd;
    mY += mYd;
    mZ += mZd;

    double horizontal = sqrt(mXd * mXd + mZd * mZd);
    mYRot = (float)(atan2(mXd, mZd) * Mth::RAD_TO_DEG);
    mXRot = (float)(atan2(mYd, horizontal) * Mth::RAD_TO_DEG);

    while (mXRot - mXRotO < -180.0f)
    {
        mXRotO -= 360.0f;
    }

    while (mXRot - mXRotO >= 180.0f)
    {
        mXRotO += 360.0f;
    }

    while (mYRot - mYRotO < -180.0f)
    {
        mYRotO -= 360.0f;
    }

    while (mYRot - mYRotO >= 180.0f)
    {
        mYRotO += 360.0f;
    }

    mXRot = mXRotO + (mXRot - mXRotO) * 0.2f;
    mYRot = mYRotO + (mYRot - mYRotO) * 0.2f;

    if (!mLevel->mIsClientSide)
    {
        double dx = mTargetX - mX;
        double dz = mTargetZ - mZ;
        double distance = sqrt(dx * dx + dz * dz);
        float angle = (float)atan2(dz, dx);
        double speed = horizontal + (distance - horizontal) * 0.0025;

        if (distance < 1.0)
        {
            speed *= 0.8;
            mYd *= 0.8;
        }

        mXd = cosf(angle) * speed;
        mZd = sinf(angle) * speed;

        if (mY < mTargetY)
        {
            mYd += (1.0 - mYd) * 0.015;
        }
        else
        {
            mYd += (-1.0 - mYd) * 0.015;
        }
    }

    if (isInWater())
    {
        for (int i = 0; i < 4; ++i)
        {
            mLevel->addParticle(PARTICLE_BUBBLE, mX - mXd * 0.25, mY - mYd * 0.25, mZ - mZd * 0.25, mXd, mYd, mZd,
                                arrayWithLength<int>());
        }
    }
    else
    {
        mLevel->addParticle(PARTICLE_ENDER, mX - mXd * 0.25 + mRandom->nextDouble() * 0.6 - 0.3, mY - mYd * 0.25 - 0.5,
                            mZ - mZd * 0.25 + mRandom->nextDouble() * 0.6 - 0.3, mXd, mYd, mZd, arrayWithLength<int>());
    }

    if (mLevel->mIsClientSide)
    {
        return;
    }

    setPos(mX, mY, mZ);

    ++mLife;

    if (mLife > 80 && !mLevel->mIsClientSide)
    {
        playSound(SoundEvents::ENDEREYE_DEATH, 1.0f, 1.0f);
        remove();

        if (mDropItem)
        {
            mLevel->addEntity(boost::shared_ptr<ItemEntity>(
                new ItemEntity(mLevel, mX, mY, mZ, not_null_ptr<ItemInstance>(new ItemInstance(Items::enderEye)))));
        }
        else
        {
            mLevel->levelEvent(LevelEvent::PARTICLES_EYE_OF_ENDER_DEATH, BlockPos(shared_from_this()), 0);
        }
    }
}

void EyeOfEnderSignal::addAdditonalSaveData(CompoundTag*)
{
}

void EyeOfEnderSignal::readAdditionalSaveData(CompoundTag*)
{
}

float EyeOfEnderSignal::getBrightness()
{
    return 1.0;
}

int EyeOfEnderSignal::getLightColor()
{
    return 0xF000F0;
}

bool EyeOfEnderSignal::isAttackable()
{
    return false;
}

Entity* EyeOfEnderSignal::create(Level* level)
{
    return new EyeOfEnderSignal(level);
}

unsigned long long EyeOfEnderSignal::GetType()
{
    return EntityType::EYE_OF_ENDER_SIGNAL;
}
