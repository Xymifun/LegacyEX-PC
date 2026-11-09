#include "game/core/Slime.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/util/AABB.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/ParticleType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/JumpControl.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/MobNearestMobTargetGoal.hpp"
#include "game/world/generation/MobNearestPlayerTargetGoal.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Difficulty.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/core/PIXNamedEvents.hpp"

const EntityDataAccessor<int>* Slime::DATA_ID_SIZE = NULL;

Slime::Slime(Level* level) : Mob(level)
{
    fjDerivedCtorCalls();
    _init();
    delete mMoveControl;
    mMoveControl = new SlimeMoveControl(this);
}

unsigned long long Slime::GetType()
{
    return EntityType::SLIME;
}

void Slime::staticCtor()
{
    DATA_ID_SIZE = SynchedEntityData::defineId<int>(EntityType::SLIME, EntityType::MOB, EntityDataSerializers::INT);
}

Entity* Slime::create(Level* level)
{
    return new Slime(level);
}

void Slime::_init()
{
    mWasOnGround = false;
    mTargetSquish = mSquish = mSquishO = 0.0f;
}

void Slime::registerGoals()
{
    mGoalSelector.addGoal(1, new SlimeFloatGoal(this), true);
    mGoalSelector.addGoal(2, new SlimeAttackGoal(this), true);
    mGoalSelector.addGoal(3, new SlimeRandomDirectionGoal(this), true);
    mGoalSelector.addGoal(5, new SlimeKeepOnJumpingGoal(this), true);
    mTargetSelector.addGoal(1, new MobNearestPlayerTargetGoal(this), true);
    mTargetSelector.addGoal(3, new MobNearestMobTargetGoal(this, typeid(VillagerGolem)), true);
}

void Slime::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_SIZE, 1);
}

void Slime::setSize(int size, bool reset)
{
    mEntityData->set(DATA_ID_SIZE, size, true);
    setSize(0.51000005f * (float)size, 0.51000005f * (float)size);
    setPos(mX, mY, mZ);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue((double)(size * size));
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue((double)(0.1f * (float)size + 0.2f));
    if (reset)
        setHealth(getMaxHealth());
    mExperienceReward = size;
}

int Slime::getSize()
{
    return mEntityData->get(DATA_ID_SIZE);
}

void Slime::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::SLIME);
}

void Slime::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"Size", getSize() - 1);
    tag->putBoolean(L"wasOnGround", mWasOnGround);
}

void Slime::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    int size = tag->getInt(L"Size");
    if (size < 0)
        size = 0;
    setSize(size + 1, false);
    mWasOnGround = tag->getBoolean(L"wasOnGround");
}

bool Slime::isTiny()
{
    return getSize() <= 1;
}

const ParticleType* Slime::getParticleType()
{
    return PARTICLE_SLIME;
}

void Slime::tick()
{
    PIXBeginNamedEvent(0.0f, "Slime tick");

    if (!mLevel->mIsClientSide && mLevel->getDifficulty() == Difficulty::PEACEFUL && getSize() > 0)
    {
        mRemoved = true;
    }

    mSquish += (mTargetSquish - mSquish) * 0.5f;
    mSquishO = mSquish;
    Mob::tick();

    if (mOnGround && !mWasOnGround)
    {
        int size = getSize();

        for (int i = 0; i < size * 8; ++i)
        {
            float angle = mRandom->nextFloat() * 6.2831855f;
            float distance = mRandom->nextFloat() * 0.5f + 0.5f;
            float xOffset = Mth::sin(angle) * (float)size * 0.5f * distance;
            float zOffset = Mth::cos(angle) * (float)size * 0.5f * distance;
            mLevel->addParticle(getParticleType(), mX + xOffset, mBoundingBox->mMinY, mZ + zOffset, 0.0, 0.0, 0.0,
                                arrayWithLength<int>());
        }

        playSound(getSquishSound(), getSoundVolume(),
                  ((mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f) / 0.8f);
        mTargetSquish = -0.5f;
    }
    else if (!mOnGround && mWasOnGround)
    {
        mTargetSquish = 1.0f;
    }

    mWasOnGround = mOnGround;
    decreaseSquish();

    if (mLevel->mIsClientSide)
    {
        float scale = 0.6f * (float)getSize();
        setSize(scale, scale);
    }

    PIXEndNamedEvent();
}

void Slime::decreaseSquish()
{
    mTargetSquish *= 0.6f;
}

int Slime::getJumpDelay()
{
    return mRandom->nextInt(20) + 10;
}

boost::shared_ptr<Slime> Slime::createChild()
{
    return boost::shared_ptr<Slime>(new Slime(mLevel));
}

void Slime::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_ID_SIZE->getId() == accessor->getId())
    {
        float f = (float)getSize() * 0.51000005f;
        setSize(f, f);
        mYRot = mYHeadRot;
        mYBodyRot = mYHeadRot;
        if (isInWater() && mRandom->nextInt(20) == 0)
            doWaterSplashEffect();
    }
    LivingEntity::onSyncedDataUpdated(accessor);
}

void Slime::remove()
{
    int size = getSize();
    if (!mLevel->mIsClientSide && size > 1 && getHealth() <= 0.0f)
    {
        int count = 2 + mRandom->nextInt(3);
        for (int i = 0; i < count; ++i)
        {
            if (i != 0)
            {
                unsigned int slimeCount = mLevel->countInstanceOf(EntityType::SLIME, true, NULL, NULL);
                if (slimeCount >= MobCategory::SLIME->getMaxInstancesPerLevel())
                    continue;
            }

            float offsetX = (static_cast<float>(i % 2) - 0.5f) * static_cast<float>(size) * 0.25f;
            float offsetZ = (static_cast<float>(i / 2) - 0.5f) * static_cast<float>(size) * 0.25f;
            boost::shared_ptr<Slime> child = createChild();
            if (hasCustomName())
            {
                child->setCustomName(getCustomName(), getOwnerUUID(), wasNamedByRestrictedPlayer());
            }
            else
            {
                child->setPersistenceRequired(true);
            }
            child->setSize(size / 2, true);
            child->moveTo(mX + offsetX, mY + 0.5, mZ + offsetZ, mRandom->nextFloat() * 360.0f, 0.0f);
            mLevel->addEntity(child);
        }
    }
    Entity::remove();
}

void Slime::push(boost::shared_ptr<Entity> entity)
{
    Entity::push(entity);
    bool isGolem = eTYPE_DERIVED_FROM(EntityType::VILLAGER_GOLEM, entity->GetType());
    if (isGolem && isDealsDamage())
        dealDamage(boost::static_pointer_cast<LivingEntity>(entity));
}

void Slime::playerTouch(boost::shared_ptr<Player> player)
{
    if (isDealsDamage())
        dealDamage(player);
}

void Slime::dealDamage(boost::shared_ptr<LivingEntity> entity)
{
    int size = getSize();
    if (canSee(entity) && distanceToSqr(entity) < (0.6 * (double)size) * (0.6 * (double)size))
    {
        EntityDamageSource* source = DamageSource::CreateMobAttack(boost::static_pointer_cast<Mob>(shared_from_this()));
        if (entity->hurt(source, (float)getAttackDamage()))
        {
            playSound(SoundEvents::SLIME_ATTACK, 1.0f, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
            doEnchantDamageEffects(boost::static_pointer_cast<LivingEntity>(shared_from_this()), entity);
        }
        delete source;
    }
}

float Slime::getEyeHeight()
{
    return 0.625f * mBbHeight;
}

bool Slime::isDealsDamage()
{
    return !isTiny();
}

int Slime::getAttackDamage()
{
    return getSize();
}

const SoundEvent* Slime::getHurtSound(const DamageSource*)
{
    return isTiny() ? SoundEvents::SMALL_SLIME_HURT : SoundEvents::SLIME_HURT;
}

const SoundEvent* Slime::getDeathSound()
{
    return isTiny() ? SoundEvents::SMALL_SLIME_DEATH : SoundEvents::SLIME_DEATH;
}

const SoundEvent* Slime::getSquishSound()
{
    return isTiny() ? SoundEvents::SMALL_SLIME_SQUISH : SoundEvents::SLIME_SQUISH;
}

ResourceLocation* Slime::getDefaultLootTable()
{
    return getSize() == 1 ? &BuiltInLootTables::SLIME : BuiltInLootTables::EMPTY;
}

bool Slime::checkSpawnRules()
{
    BlockPos pos(Mth::floor(mX), 0, Mth::floor(mZ));
    LevelChunk* chunk = mLevel->getChunkAt(pos);
    if ((mLevel->getLevelData()->getGeneratorType() != LevelType::FLAT || mRandom->nextInt(4) == 1)
        && mLevel->getDifficulty() != Difficulty::PEACEFUL)
    {
        Biome* biome = mLevel->getBiome(pos);
        if (biome == Biome::SWAMPLAND && mY > 50.0 && mY < 70.0 && mRandom->nextFloat() < 0.5f)
        {
            if (mRandom->nextFloat() < mLevel->getMoonBrightness())
            {
                if (mLevel->getRawBrightness(BlockPos(this)) <= mRandom->nextInt(8))
                    return Mob::checkSpawnRules();
            }
        }
        if (mRandom->nextInt(10) == 0)
        {
            if (mY < 40.0 && (chunk->getBiomePresentFlag() & LevelChunk::BIOME_PRESENT_SLIME_CHUNK) != 0)
                return Mob::checkSpawnRules();
        }
    }
    return false;
}

float Slime::getSoundVolume()
{
    return 0.4f * (float)getSize();
}

int Slime::getMaxHeadXRot()
{
    return 0;
}

bool Slime::doPlayJumpSound()
{
    return getSize() > 0;
}

void Slime::jumpFromGround()
{
    mYd = 0.42f;
    mHasImpulse = true;
}

MobGroupData* Slime::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                      const std::wstring& dataString)
{
    int sizeIndex = mRandom->nextInt(3);
    if (sizeIndex < 2 && mRandom->nextFloat() < 0.5f * difficulty.getSpecialMultiplier())
        ++sizeIndex;
    int size = 1 << sizeIndex;
    setSize(size, true);
    return Mob::finalizeMobSpawn(difficulty, data, L"");
}

const SoundEvent* Slime::getJumpSound()
{
    return isTiny() ? SoundEvents::SMALL_SLIME_JUMP : SoundEvents::SLIME_JUMP;
}

Slime::SlimeAttackGoal::SlimeAttackGoal(Slime* slime) : mSlime(slime), mGrowTiredTimer(0)
{
    setRequiredControlFlags(2);
}

Slime::SlimeAttackGoal::~SlimeAttackGoal()
{
}

bool Slime::SlimeAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mSlime->getTarget();
    if (!target)
        return false;
    if (!target->isAlive())
        return false;
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType())
        && boost::static_pointer_cast<Player>(target)->mAbilities.mInvulnerable)
        return false;
    return true;
}

void Slime::SlimeAttackGoal::start()
{
    mGrowTiredTimer = 300;
    Goal::start();
}

bool Slime::SlimeAttackGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mSlime->getTarget();
    if (!target)
        return false;
    if (!target->isAlive())
        return false;
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType())
        && boost::static_pointer_cast<Player>(target)->mAbilities.mInvulnerable)
        return false;
    if (--mGrowTiredTimer <= 0)
        return false;
    return true;
}

void Slime::SlimeAttackGoal::tick()
{
    mSlime->lookAt(mSlime->getTarget(), 10.0f, 10.0f);
    static_cast<SlimeMoveControl*>(mSlime->getMoveControl())->setDirection(mSlime->mYRot, mSlime->isDealsDamage());
}

Slime::SlimeFloatGoal::SlimeFloatGoal(Slime* slime) : mSlime(slime)
{
    setRequiredControlFlags(5);
    static_cast<GroundPathNavigation*>(slime->getNavigation())->setCanFloat(true);
}

Slime::SlimeFloatGoal::~SlimeFloatGoal()
{
}

bool Slime::SlimeFloatGoal::canUse()
{
    return mSlime->isInWater() || mSlime->isInLava();
}

void Slime::SlimeFloatGoal::tick()
{
    if (mSlime->getRandom()->nextFloat() < 0.8f)
        mSlime->getJumpControl()->jump();
    static_cast<SlimeMoveControl*>(mSlime->getMoveControl())->setWantedMovement(1.2);
}

Slime::SlimeKeepOnJumpingGoal::SlimeKeepOnJumpingGoal(Slime* slime) : mSlime(slime)
{
    setRequiredControlFlags(5);
}

Slime::SlimeKeepOnJumpingGoal::~SlimeKeepOnJumpingGoal()
{
}

bool Slime::SlimeKeepOnJumpingGoal::canUse()
{
    return true;
}

void Slime::SlimeKeepOnJumpingGoal::tick()
{
    static_cast<SlimeMoveControl*>(mSlime->getMoveControl())->setWantedMovement(1.0);
}

Slime::SlimeMoveControl::SlimeMoveControl(Slime* slime)
    : MoveControl(slime),
      mYRot(180.0f * slime->mYRot / Mth::PI),
      mSlime(slime)
{
    mJumpDelay = 0;
    mWasOnGround = false;
}

Slime::SlimeMoveControl::~SlimeMoveControl()
{
}

void Slime::SlimeMoveControl::setDirection(float yRot, bool jump)
{
    mYRot = yRot;
    mWasOnGround = jump;
}

void Slime::SlimeMoveControl::setWantedMovement(double speed)
{
    mSpeedModifier = speed;
    mOperation = MOVE_TO;
}

void Slime::SlimeMoveControl::tick()
{
    float yRot = rotlerp(mMob->mYRot, mYRot, 30.0f);
    mMob->mYRot = mMob->mYBodyRot = mMob->mYHeadRot = yRot;
    if (mOperation != MOVE_TO)
    {
        mMob->setZza(0.0f);
        return;
    }
    mOperation = WAIT;
    if (mMob->mOnGround)
    {
        mMob->setSpeed(
            (float)(mSpeedModifier * mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
        if (mJumpDelay-- <= 0)
        {
            int jumpDelay = mSlime->getJumpDelay();
            if (mWasOnGround)
                jumpDelay /= 3;
            mJumpDelay = jumpDelay;
            static_cast<JumpControl*>(mSlime->getJumpControl())->jump();
            if (mSlime->doPlayJumpSound())
            {
                mSlime->playSound(mSlime->getJumpSound(), mSlime->getSoundVolume(),
                                  ((mSlime->getRandom()->nextFloat() - mSlime->getRandom()->nextFloat()) * 0.2f + 1.0f)
                                      * 0.8f);
            }
        }
        else
        {
            mSlime->mZza = mSlime->mXxa = 0.0f;
            mMob->setSpeed(0.0f);
        }
    }
    else
    {
        mMob->setSpeed(
            (float)(mSpeedModifier * mMob->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
    }
}

Slime::SlimeRandomDirectionGoal::SlimeRandomDirectionGoal(Slime* slime)
    : mSlime(slime),
      mChosenDegrees(0.0f),
      mChosenTicks(0)
{
    setRequiredControlFlags(2);
}

Slime::SlimeRandomDirectionGoal::~SlimeRandomDirectionGoal()
{
}

bool Slime::SlimeRandomDirectionGoal::canUse()
{
    return !mSlime->getTarget()
           && (mSlime->mOnGround || mSlime->isInWater() || mSlime->isInLava()
               || mSlime->hasEffect(MobEffects::LEVITATION));
}

void Slime::SlimeRandomDirectionGoal::tick()
{
    if (--mChosenTicks <= 0)
    {
        mChosenTicks = mSlime->getRandom()->nextInt(60) + 40;
        mChosenDegrees = (float)mSlime->getRandom()->nextInt(360);
    }
    static_cast<SlimeMoveControl*>(mSlime->getMoveControl())->setDirection(mChosenDegrees, false);
}
