#include "game/core/Blaze.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <typeinfo>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/projectile/SmallFireball.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

static const EntityDataAccessor<unsigned char>* DATA_FLAGS = NULL;

Blaze::Blaze(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    setPathfindingMalus(BlockPathTypes::WATER, -1.0f);
    setPathfindingMalus(BlockPathTypes::LAVA, 8.0f);
    setPathfindingMalus(BlockPathTypes::DANGER_FIRE, 0.0f);
    setPathfindingMalus(BlockPathTypes::DAMAGE_FIRE, 0.0f);
    mFireImmune = true;
    mExperienceReward = 10;
    mHeightOffset = 0.5f;
    mHeightOffsetUpdateTime = 0;
    mUnused = 0;
}

void Blaze::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId<unsigned char>(EntityType::BLAZE, EntityType::MONSTER,
                                                            EntityDataSerializers::BYTE);
}

void Blaze::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::BLAZE);
}

void Blaze::registerGoals()
{
    mGoalSelector.addGoal(4, new BlazeAttackGoal(this), true);
    mGoalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new WaterAvoidingRandomStrollGoal(this, 1.0, 0.0f), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
}

void Blaze::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23f);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(48.0);
}

void Blaze::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
}

const SoundEvent* Blaze::getAmbientSound()
{
    return SoundEvents::BLAZE_AMBIENT;
}

const SoundEvent* Blaze::getHurtSound(const DamageSource*)
{
    return SoundEvents::BLAZE_HURT;
}

const SoundEvent* Blaze::getDeathSound()
{
    return SoundEvents::BLAZE_DEATH;
}

int Blaze::getLightColor()
{
    return 0xF000F0;
}

float Blaze::getBrightness()
{
    return 1.0f;
}

void Blaze::aiStep()
{
    if (!mOnGround && mYd < 0.0)
    {
        mYd *= 0.6;
    }

    if (mLevel->mIsClientSide)
    {
        if (mRandom->nextInt(24) == 0 && !isSilent())
        {
            mLevel->playLocalSound(mX + 0.5, mY + 0.5, mZ + 0.5, SoundEvents::BLAZE_BURN, getSoundSource(),
                                   1.0f + mRandom->nextFloat(), 0.3f + mRandom->nextFloat() * 0.7f, false, 16.0f);
        }

        for (int i = 0; i < 2; ++i)
        {
            mLevel->addParticle(PARTICLE_LARGE_SMOKE, mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
                                mY + mRandom->nextDouble() * mBbHeight, mZ + (mRandom->nextDouble() - 0.5) * mBbWidth,
                                0.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }

    Monster::aiStep();
}

void Blaze::customServerAiStep()
{
    if (isInWaterRainOrBubble())
    {
        hurt(DamageSource::DROWN, 1.0f);
    }

    if (--mHeightOffsetUpdateTime <= 0)
    {
        mHeightOffsetUpdateTime = 100;
        mHeightOffset = static_cast<float>(mRandom->nextGaussian()) * 3.0f + 0.5f;
    }

    boost::shared_ptr<LivingEntity> target = getTarget();
    if (target && target->mY + target->getEyeHeight() > mY + getEyeHeight() + mHeightOffset)
    {
        mYd += (0.3f - mYd) * 0.3f;
        mHasImpulse = true;
    }

    Mob::customServerAiStep();
}

void Blaze::causeFallDamage(float, float)
{
}

ResourceLocation* Blaze::getDefaultLootTable()
{
    return &BuiltInLootTables::BLAZE;
}

bool Blaze::isCharged()
{
    return (mEntityData->get(DATA_FLAGS) & 1) != 0;
}

bool Blaze::isOnFire()
{
    return Blaze::isCharged();
}

void Blaze::dropDeathLoot(bool hitByPlayer, int lootingLevel)
{
    if (!hitByPlayer)
    {
        return;
    }

    int rods = mRandom->nextInt(lootingLevel + 2);
    for (int i = 0; i < rods; ++i)
    {
        spawnAtLocation(Items::blazeRod, 1);
    }

    int dust = mRandom->nextInt(lootingLevel + 3);
    for (int i = 0; i < dust; ++i)
    {
        spawnAtLocation(Items::glowstoneDust, 1);
    }
}

void Blaze::setCharged(bool charged)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    flags = charged ? static_cast<unsigned char>(flags | 1) : static_cast<unsigned char>(flags & 0xFE);
    mEntityData->set(DATA_FLAGS, flags, true);
}

bool Blaze::isDarkEnoughToSpawn()
{
    return true;
}

double Blaze::getFollowDistance()
{
    AttributeInstance* attribute = getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    return attribute == NULL ? 16.0 : attribute->getValue();
}

unsigned long long Blaze::GetType()
{
    return EntityType::BLAZE;
}

Entity* Blaze::create(Level* level)
{
    return new Blaze(level);
}

Blaze::BlazeAttackGoal::BlazeAttackGoal(Blaze* blaze) : Goal(), mBlaze(blaze)
{
    setRequiredControlFlags(3);
    mAttackTime = 0;
}

bool Blaze::BlazeAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mBlaze->getTarget();
    return target != NULL && target->isAlive();
}

void Blaze::BlazeAttackGoal::start()
{
    mAttackStep = 0;
}

void Blaze::BlazeAttackGoal::stop()
{
    mBlaze->setCharged(false);
}

void Blaze::BlazeAttackGoal::tick()
{
    --mAttackTime;
    boost::shared_ptr<LivingEntity> target = mBlaze->getTarget();
    double distance = mBlaze->distanceToSqr(target);
    if (distance < mBlaze->getFollowDistance() * mBlaze->getFollowDistance())
    {
        mBlaze->getLookControl()->setLookAt(target, 10.0f, 10.0f);
    }

    if (distance < 4.0)
    {
        if (mAttackTime <= 0)
        {
            mAttackTime = 20;
            mBlaze->doHurtTarget(target);
        }
        mBlaze->getMoveControl()->setWantedPosition(target->mX, target->mY, target->mZ, 1.0);
    }
    else if (distance < 256.0)
    {
        double xd = target->mX - mBlaze->mX;
        double yd = target->mBoundingBox->mMinY + target->mBbHeight * 0.5f - (mBlaze->mY + mBlaze->mBbHeight * 0.5f);
        double zd = target->mZ - mBlaze->mZ;

        if (mAttackTime <= 0)
        {
            ++mAttackStep;
            if (mAttackStep == 1)
            {
                mAttackTime = 60;
                mBlaze->setCharged(true);
            }
            else if (mAttackStep <= 4)
            {
                mAttackTime = 6;
            }
            else
            {
                mAttackTime = 100;
                mAttackStep = 0;
                mBlaze->setCharged(false);
            }

            if (mAttackStep > 1)
            {
                Random* random = mBlaze->mRandom;
                float spread = static_cast<float>(std::sqrt(std::sqrt(distance))) * 0.5f;
                mBlaze->mLevel->levelEvent(
                    boost::shared_ptr<Player>(), LevelEvent::SOUND_BLAZE_FIREBALL,
                    BlockPos(static_cast<int>(mBlaze->mX), static_cast<int>(mBlaze->mY), static_cast<int>(mBlaze->mZ)),
                    0);
                for (int i = 0; i < 1; ++i)
                {
                    boost::shared_ptr<SmallFireball> fireball(new SmallFireball(
                        mBlaze->mLevel, boost::dynamic_pointer_cast<Mob>(mBlaze->shared_from_this()),
                        xd + random->nextGaussian() * spread, yd, zd + random->nextGaussian() * spread));
                    fireball->mY = mBlaze->mY + mBlaze->mBbHeight * 0.5f + 0.5;
                    mBlaze->mLevel->addEntity(fireball);
                }
            }
        }

        mBlaze->getLookControl()->setLookAt(target, 10.0f, 10.0f);
    }
}
