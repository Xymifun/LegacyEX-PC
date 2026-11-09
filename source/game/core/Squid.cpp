#include "game/core/Squid.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/item/ItemId.hpp"

Squid::Squid(Level* level) : WaterAnimal(level)
{
    Mob::fjDerivedCtorCalls();
    _init();
    setSize(0.8f, 0.8f);
    mRandom->setSeed((long long)(getId() + 1));
    mTentacleSpeed = 1.0f / (mRandom->nextFloat() + 1.0f) * 0.2f;
}

Entity* Squid::create(Level* level)
{
    return new Squid(level);
}

void Squid::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::SQUID);
}

void Squid::_init()
{
    mXBodyRotO = 0.0f;
    mXBodyRot = 0.0f;
    mZBodyRotO = 0.0f;
    mZBodyRot = 0.0f;
    mOldTentacleMovement = 0.0f;
    mTentacleMovement = 0.0f;
    mOldTentacleAngle = 0.0f;
    mTentacleAngle = 0.0f;
    mSpeed = 0.0f;
    mTentacleSpeed = 0.0f;
    mRotateSpeed = 0.0f;
    mMovementZ = 0.0f;
    mMovementY = 0.0f;
    mMovementX = 0.0f;
}

void Squid::registerGoals()
{
    mGoalSelector.addGoal(0, new SquidRandomMovementGoal(this), true);
}

void Squid::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(10.0);
}

float Squid::getEyeHeight()
{
    return static_cast<double>(mBbHeight * 0.5f);
}

const SoundEvent* Squid::getAmbientSound()
{
    return SoundEvents::SQUID_AMBIENT;
}

const SoundEvent* Squid::getHurtSound(const DamageSource*)
{
    return SoundEvents::SQUID_HURT;
}

const SoundEvent* Squid::getDeathSound()
{
    return SoundEvents::SQUID_DEATH;
}

float Squid::getSoundVolume()
{
    return 0.4f;
}

bool Squid::makeStepSound()
{
    return 1;
}

void Squid::playStepSound(const BlockPos&, Block*)
{
}

ResourceLocation* Squid::getDefaultLootTable()
{
    return &BuiltInLootTables::SQUID;
}

void Squid::aiStep()
{
    Mob::aiStep();
    mXBodyRotO = mXBodyRot;
    mZBodyRotO = mZBodyRot;
    mOldTentacleMovement = mTentacleMovement;
    mOldTentacleAngle = mTentacleAngle;
    mTentacleMovement += mTentacleSpeed;
    if (mTentacleMovement > 6.2831855f)
    {
        mTentacleMovement -= 6.2831855f;
        if (!mLevel->mIsClientSide)
        {
            if (mRandom->nextInt(10) == 0)
            {
                mTentacleSpeed = 1.0f / (mRandom->nextFloat() + 1.0f) * 0.2f;
            }
            mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::SQUID_ANIM_SYNCH, 0);
        }
    }

    if (isInWaterOrBubble())
    {
        if (mTentacleMovement < 3.1415927f)
        {
            float progress = mTentacleMovement / 3.1415927f;
            mTentacleAngle = Mth::sin(progress * progress * 3.1415927f) * 3.1415927f * 0.25f;
            if ((double)progress > 0.75)
            {
                mSpeed = 1.0f;
                mRotateSpeed = 1.0f;
            }
            else
            {
                mRotateSpeed *= 0.8f;
            }
        }
        else
        {
            mTentacleAngle = 0.0f;
            mSpeed *= 0.9f;
            mRotateSpeed *= 0.99f;
        }

        if (!mLevel->mIsClientSide)
        {
            mXd = (double)(mMovementX * mSpeed);
            mYd = (double)(mMovementY * mSpeed);
            mZd = (double)(mMovementZ * mSpeed);
        }
        double horizontalSpeed = std::sqrt(mXd * mXd + mZd * mZd);
        mYBodyRot += (-(float)std::atan2(mXd, mZd) * 57.295776f - mYBodyRot) * 0.1f;
        mYRot = mYBodyRot;
        mZBodyRot += 3.1415927f * mRotateSpeed * 1.5f;
        mXBodyRot += (-(float)std::atan2(horizontalSpeed, mYd) * 57.295776f - mXBodyRot) * 0.1f;
    }
    else
    {
        mTentacleAngle = Mth::abs(Mth::sin(mTentacleMovement)) * 3.1415927f * 0.25f;
        mXd = 0.0;
        mZd = 0.0;
        if (hasEffect(MobEffects::LEVITATION))
        {
            mYd = (mYd + (0.05 * (double)(getEffect(MobEffects::LEVITATION)->getAmplifier() + 1) - mYd)) * 0.98f;
        }
        else
        {
            if (!isNoGravity())
            {
                mYd -= 0.08;
            }
            mYd *= 0.98f;
        }
        mXBodyRot += (-90.0f - mXBodyRot) * 0.02f;
    }
}

void Squid::travel(float xa, float ya, float za)
{
    move(eMoverType_Self, mXd, mYd, mZd, false);
}

bool Squid::checkSpawnRules()
{
    return mY > 45.0 && mY < static_cast<double>(mLevel->getSeaLevel()) && WaterAnimal::checkSpawnRules();
}

Vec3* Squid::randomInkDir()
{
    Random* random = getRandom();
    Vec3* backwards = getViewVector(0.0f)->multiply(-0.25, -0.25, -0.25);
    float xSpread = random->nextFloat() * mBbWidth;
    float x = (float)backwards->x + (xSpread + xSpread) - mBbWidth;
    float yBase = (float)backwards->y + 0.5f;
    float y = random->nextFloat() * mBbHeight + yBase;
    float zSpread = random->nextFloat() * mBbWidth;
    float z = (float)backwards->z + (zSpread + zSpread) - mBbWidth;
    return Vec3::newTemp(x, y, z)->normalize();
}

void Squid::spawnInkParticles()
{
    int count = 15;
    int inkValue = 500;
    if (isBaby())
    {
        count = 8;
        inkValue = 300;
    }
    for (int i = 0; i < count; ++i)
    {
        Vec3* direction = randomInkDir()->multiply(0.1, 0.1, 0.1);
        int inkData[1] = {inkValue};
        mLevel->addParticle(PARTICLE_INK, mX, mY, mZ, direction->x, direction->y, direction->z,
                            arrayWithLength<int>(inkData, 1));
    }

    for (int i = 0; i < 5; ++i)
    {
        Vec3* direction = randomInkDir();
        int bubbleData[4] = {0, 1, 244, 1};
        mLevel->addParticle(PARTICLE_BUBBLE, mX, mY, mZ, direction->x, direction->y, direction->z,
                            arrayWithLength<int>(bubbleData, 4));
    }
}

void Squid::handleEntityEvent(unsigned char event, int data)
{
    switch (event)
    {
    case EntityEvent::SQUID_INK:
        spawnInkParticles();
        mLevel->playLocalSound(mX, mY, mZ, SoundEvents::SQUID_SQUIRT, SoundSource::eSoundSource_Hostile, 0.3f, 1.0f,
                               false, 16.0f);
        break;
    case EntityEvent::SQUID_ANIM_SYNCH:
        mTentacleMovement = 0;
        break;
    default:
        Mob::handleEntityEvent(event, data);
        break;
    }
}

void Squid::setMovementVector(float x, float y, float z)
{
    mMovementX = x;
    mMovementY = y;
    mMovementZ = z;
}

bool Squid::hasMovementVector()
{
    return mMovementX != 0.0f || mMovementY != 0.0f || mMovementZ != 0.0f;
}

bool Squid::hurt(const DamageSource* source, float damage)
{
    bool hurt = LivingEntity::hurt(source, damage);
    if (source->getEntity() != NULL)
    {
        spawnInkParticles();
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::SQUID_INK, 0);
    }
    else
    {
        if (getJumpControl() != NULL)
            getJumpControl()->jump();
    }
    return hurt;
}

unsigned int Squid::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::LEAD && !isLeashed())
    {
        return IDS_TOOLTIPS_LEASH;
    }
    return Mob::GetInteractTooltip(holder);
}

unsigned long long Squid::GetType()
{
    return EntityType::SQUID;
}

Squid::SquidRandomMovementGoal::SquidRandomMovementGoal(Squid* squid) : Goal(), mSquid(squid)
{
}

Squid::SquidRandomMovementGoal::~SquidRandomMovementGoal()
{
}

bool Squid::SquidRandomMovementGoal::canUse()
{
    return true;
}

void Squid::SquidRandomMovementGoal::tick()
{
    if (mSquid->getNoActionTime() > 100)
    {
        mSquid->setMovementVector(0.0f, 0.0f, 0.0f);
    }
    else if (mSquid->getRandom()->nextInt(50) == 0 || !mSquid->mIsInWater || !mSquid->hasMovementVector())
    {
        float angle = mSquid->getRandom()->nextFloat() * 6.2831855f;
        float x = Mth::cos(angle) * 0.2f;
        float y = -0.1f + mSquid->getRandom()->nextFloat() * 0.2f;
        float z = Mth::sin(angle) * 0.2f;
        mSquid->setMovementVector(x, y, z);
    }
}
