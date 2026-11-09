#include "game/core/Guardian.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"

#include <cmath>
#include <typeinfo>

#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/ai/WaterBoundPathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"

const EntityDataAccessor<int>* Guardian::DATA_ID_ATTACK_TARGET = NULL;

Guardian::Guardian(Level* level, bool createEntityData) : Monster(level), mCachedAttackTarget()
{
    if (createEntityData)
    {
        fjDerivedCtorCalls();
    }
    mTailAnimationSpeed = 0.0f;
    mTailAnimationOld = 0.0f;
    mTailAnimation = 0.0f;
    mSpikesAnimationOld = 0.0f;
    mSpikesAnimation = 0.0f;
    mCachedAttackTarget = boost::shared_ptr<LivingEntity>();
    mExperienceReward = 10;
    mTouchedGround = false;
    mAttackTime = 0;
    setSize(0.85f, 0.85f);
    delete mMoveControl;
    mMoveControl = new GuardianMoveControl(this);
    mTailAnimation = mRandom->nextFloat();
    mSpikesAnimationOld = mTailAnimation;
}

Guardian::~Guardian()
{
}

void Guardian::staticCtor()
{
    DATA_ID_ATTACK_TARGET = SynchedEntityData::defineId(EntityType::GUARDIAN, EntityType::MONSTER,
                                                        EntityDataSerializers::INT);
}

void Guardian::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::GUARDIAN);
}

Entity* Guardian::create(Level* level)
{
    return new Guardian(level, true);
}

void Guardian::registerGoals()
{
    MoveTowardsRestrictionGoal* restriction = new MoveTowardsRestrictionGoal(this, 1.0);
    mRandomStrollGoal = new RandomStrollGoal(this, 1.0, 80);
    mGoalSelector.addGoal(4, new GuardianAttackGoal(this), true);
    mGoalSelector.addGoal(5, restriction, true);
    mGoalSelector.addGoal(7, mRandomStrollGoal, true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Guardian), 12.0f, 0.01f), true);
    mGoalSelector.addGoal(9, new RandomLookAroundGoal(this), true);
    mRandomStrollGoal->setRequiredControlFlags(3);
    restriction->setRequiredControlFlags(3);
    mTargetSelector.addGoal(1,
                            new NearestAttackableTargetGoal(this, typeid(LivingEntity), 10, true, false,
                                                            new GuardianAttackSelector(this), true, true),
                            true);
}

void Guardian::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(16.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30.0);
}

PathNavigation* Guardian::createNavigation(Level* level)
{
    return new WaterBoundPathNavigation(this, level, true, false);
}

void Guardian::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_ATTACK_TARGET, 0);
}

unsigned int Guardian::getAttackDuration()
{
    return 80;
}

void Guardian::setActiveAttackTarget(int targetId)
{
    mEntityData->set(DATA_ID_ATTACK_TARGET, targetId, true);
}

bool Guardian::hasActiveAttackTarget()
{
    return mEntityData->get(DATA_ID_ATTACK_TARGET) != 0;
}

boost::shared_ptr<LivingEntity> Guardian::getActiveAttackTarget()
{
    if (!hasActiveAttackTarget())
    {
        return boost::shared_ptr<LivingEntity>();
    }
    if (mLevel->mIsClientSide)
    {
        if (mCachedAttackTarget != NULL)
        {
            return mCachedAttackTarget;
        }
        boost::shared_ptr<Entity> target = mLevel->getEntity(mEntityData->get(DATA_ID_ATTACK_TARGET));
        if (target != NULL && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
        {
            mCachedAttackTarget = boost::static_pointer_cast<LivingEntity>(target);
            return mCachedAttackTarget;
        }
        return boost::shared_ptr<LivingEntity>();
    }
    return getTarget();
}

void Guardian::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    LivingEntity::onSyncedDataUpdated(accessor);
    if (accessor->getId() == DATA_ID_ATTACK_TARGET->getId())
    {
        mAttackTime = 0;
        mCachedAttackTarget = boost::shared_ptr<LivingEntity>();
    }
}

int Guardian::getAmbientSoundInterval()
{
    return 160;
}

const SoundEvent* Guardian::getAmbientSound()
{
    return isInWaterOrBubble() ? SoundEvents::GUARDIAN_AMBIENT : SoundEvents::GUARDIAN_AMBIENT_LAND;
}

const SoundEvent* Guardian::getHurtSound(const DamageSource*)
{
    return isInWaterOrBubble() ? SoundEvents::GUARDIAN_HURT : SoundEvents::GUARDIAN_HURT_LAND;
}

const SoundEvent* Guardian::getDeathSound()
{
    return isInWaterOrBubble() ? SoundEvents::GUARDIAN_DEATH : SoundEvents::GUARDIAN_DEATH_LAND;
}

const SoundEvent* Guardian::getFlopSound()
{
    return SoundEvents::GUARDIAN_FLOP;
}

bool Guardian::makeStepSound()
{
    return false;
}

float Guardian::getEyeHeight()
{
    return mBbHeight * 0.5f;
}

float Guardian::getWalkTargetValue(const BlockPos& pos)
{
    if (mLevel->getBlockState(pos)->getMaterial() == Material::WATER)
    {
        return mLevel->getBrightness(pos) + 9.5f;
    }
    return Monster::getWalkTargetValue(pos);
}

float Guardian::getAttackAnimationScale(float partialTicks)
{
    return (mAttackTime + partialTicks) / (float)(int)getAttackDuration();
}

void Guardian::aiStep()
{
    if (mLevel->mIsClientSide)
    {
        mTailAnimationOld = mTailAnimation;
        if (!isInWater())
        {
            mTailAnimationSpeed = 2.0f;
            if (mYd > 0.0 && mTouchedGround && !isSilent())
            {
                mLevel->playLocalSound(mX, mY, mZ, getFlopSound(), getSoundSource(), 1.0f, 1.0f, false, 16.0f);
            }
            mTouchedGround = mYd < 0.0
                             && mLevel->isSolidBlockingBlockInLoadedChunk(BlockPos(shared_from_this()).below(), false);
        }
        else if (isMoving())
        {
            if (mTailAnimationSpeed < 0.5f)
            {
                mTailAnimationSpeed = 4.0f;
            }
            else
            {
                mTailAnimationSpeed += (0.5f - mTailAnimationSpeed) * 0.1f;
            }
        }
        else
        {
            mTailAnimationSpeed += (0.125f - mTailAnimationSpeed) * 0.2f;
        }
        mTailAnimation += mTailAnimationSpeed;
        mSpikesAnimationOld = mSpikesAnimation;

        if (!isInWaterOrBubble())
        {
            mSpikesAnimation = mRandom->nextFloat();
        }
        else if (isMoving())
        {
            mSpikesAnimation -= mSpikesAnimation * 0.25f;
        }
        else
        {
            mSpikesAnimation += (1.0f - mSpikesAnimation) * 0.06f;
        }

        if (isMoving() && isInWater())
        {
            Vec3* view = getViewVector(0.0f);
            for (int i = 0; i < 2; ++i)
            {
                mLevel->addParticle(PARTICLE_BUBBLE, mX + (mRandom->nextDouble() - 0.5) * mBbWidth - view->x * 1.5,
                                    mY + mRandom->nextDouble() * mBbHeight - view->y * 1.5,
                                    mZ + (mRandom->nextDouble() - 0.5) * mBbWidth - view->z * 1.5, 0.0, 0.0, 0.0,
                                    arrayWithLength<int>());
            }
        }

        if (hasActiveAttackTarget())
        {
            if (mAttackTime < (int)getAttackDuration())
            {
                ++mAttackTime;
            }
            boost::shared_ptr<LivingEntity> target = getActiveAttackTarget();
            if (target != NULL)
            {
                getLookControl()->setLookAt(target, 90.0f, 90.0f);
                getLookControl()->tick();

                float scale = getAttackAnimationScale(0.0f);
                double dx = target->mX - mX;
                double dy = (target->mY + target->mBbHeight * 0.5f) - (mY + getEyeHeight());
                double dz = target->mZ - mZ;
                double distance = sqrt(dx * dx + dy * dy + dz * dz);
                double t = mRandom->nextDouble();
                if (t < distance)
                {
                    double step = 1.7 - scale;
                    double offset = 1.8 - scale;
                    do
                    {
                        t += offset + mRandom->nextDouble() * step;
                        mLevel->addParticle(PARTICLE_BUBBLE, (dx / distance) * t + mX,
                                            (dy / distance) * t + mY + getEyeHeight(), (dz / distance) * t + mZ, 0.0,
                                            0.0, 0.0, arrayWithLength<int>());
                    } while (t < distance);
                }
            }
        }
    }

    if (isInWaterOrBubble())
    {
        setAirSupply(300);
    }
    else if (mOnGround)
    {
        mYd += 0.5;
        mXd += (mRandom->nextFloat() * 2.0f - 1.0f) * 0.4f;
        mZd += (mRandom->nextFloat() * 2.0f - 1.0f) * 0.4f;
        mYRot = mRandom->nextFloat() * 360.0f;
        mOnGround = false;
        mHasImpulse = true;
    }

    if (hasActiveAttackTarget())
    {
        mYRot = mYHeadRot;
    }

    Monster::aiStep();
}

float Guardian::getTailAnimation(float partialTicks)
{
    return mTailAnimationOld + (mTailAnimation - mTailAnimationOld) * partialTicks;
}

float Guardian::getSpikesAnimation(float partialTicks)
{
    return mSpikesAnimationOld + (mSpikesAnimation - mSpikesAnimationOld) * partialTicks;
}

ResourceLocation* Guardian::getDefaultLootTable()
{
    return &BuiltInLootTables::GUARDIAN;
}

bool Guardian::isDarkEnoughToSpawn()
{
    return true;
}

bool Guardian::checkSpawnObstruction()
{
    return mLevel->isUnobstructed(mBoundingBox, shared_from_this())
           && mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty();
}

bool Guardian::checkSpawnRules()
{
    return (mRandom->nextInt(20) == 0 || !mLevel->canSeeSkyFromBelowWater(BlockPos(shared_from_this())))
           && Monster::checkSpawnRules();
}

bool Guardian::hurt(const DamageSource* source, float damage)
{
    if (!isMoving() && !source->isMagic() && source->getDirectEntity() != NULL
        && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, source->getDirectEntity()->GetType()))
    {
        boost::shared_ptr<LivingEntity> attacker = boost::dynamic_pointer_cast<LivingEntity>(source->getDirectEntity());
        if (!source->isExplosion())
        {
            EntityDamageSource* thornsSource = DamageSource::CreateThorns(shared_from_this());
            attacker->hurt(thornsSource, 2.0f);
            delete thornsSource;
        }
    }
    if (mRandomStrollGoal != NULL)
        mRandomStrollGoal->trigger();
    return Monster::hurt(source, damage);
}

int Guardian::getMaxHeadXRot()
{
    return 180;
}

void Guardian::travel(float xxa, float yya, float zza)
{
    if (isEffectiveAi() && isInWater())
    {
        moveRelative(xxa, yya, zza, 0.1f);
        move(eMoverType_Self, mXd, mYd, mZd, false);
        mXd *= 0.8999999761581421;
        mYd *= 0.8999999761581421;
        mZd *= 0.8999999761581421;
        if (!isMoving() && getTarget() == NULL)
        {
            mYd -= 0.005;
        }
    }
    else
    {
        LivingEntity::travel(xxa, yya, zza);
    }
}

MobGroupData* Guardian::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                         const std::wstring& dataString)
{
    data = Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
    return data;
}

unsigned long long Guardian::GetType()
{
    return EntityType::GUARDIAN;
}

Guardian::GuardianAttackGoal::GuardianAttackGoal(Guardian* guardian)
    : mGuardian(guardian),
      mElder(eTYPE_DERIVED_FROM(EntityType::ELDER_GUARDIAN, guardian->GetType()))
{
    setRequiredControlFlags(3);
}

Guardian::GuardianAttackGoal::~GuardianAttackGoal()
{
}

bool Guardian::GuardianAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mGuardian->getTarget();
    return target != NULL && target->isAlive();
}

bool Guardian::GuardianAttackGoal::canContinueToUse()
{
    return Goal::canContinueToUse() && (mElder || mGuardian->distanceToSqr(mGuardian->getTarget()) > 9.0);
}

void Guardian::GuardianAttackGoal::start()
{
    mAttackTime = -10;
    mGuardian->getNavigation()->stop();
    mGuardian->getLookControl()->setLookAt(mGuardian->getTarget(), 90.0f, 90.0f);
    mGuardian->mHasImpulse = true;
}

void Guardian::GuardianAttackGoal::stop()
{
    mGuardian->setActiveAttackTarget(0);
    mGuardian->setTarget(boost::shared_ptr<LivingEntity>());
    mGuardian->mRandomStrollGoal->trigger();
}

void Guardian::GuardianAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mGuardian->getTarget();
    mGuardian->getNavigation()->stop();
    mGuardian->getLookControl()->setLookAt(target, 90.0f, 90.0f);
    if (!mGuardian->canSee(target))
    {
        mGuardian->setTarget(boost::shared_ptr<LivingEntity>());
        return;
    }
    ++mAttackTime;
    if (mAttackTime == 0)
    {
        mGuardian->setActiveAttackTarget(mGuardian->getTarget()->getId());
        mGuardian->mLevel->broadcastEntityEvent(mGuardian->shared_from_this(), EntityEvent::GUARDIAN_ATTACK_SOUND, 0);
    }
    else if (mAttackTime >= (int)mGuardian->getAttackDuration())
    {
        float damage = 1.0f;
        if (mGuardian->mLevel->getDifficulty() == Difficulty::HARD)
        {
            damage = 3.0f;
        }
        if (mElder)
        {
            damage += 2.0f;
        }
        DamageSource* indirectMagic = DamageSource::CreateIndirectMagic(
            mGuardian->shared_from_this(), mGuardian->shared_from_this(), DamageSource::MAGIC);
        EntityDamageSource* mobAttack = DamageSource::CreateMobAttack(
            boost::dynamic_pointer_cast<LivingEntity>(mGuardian->shared_from_this()));
        target->hurt(indirectMagic, damage);
        target->hurt(mobAttack, (float)mGuardian->getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue());
        delete indirectMagic;
        delete mobAttack;
        mGuardian->setTarget(boost::shared_ptr<LivingEntity>());
    }
}

Guardian::GuardianAttackSelector::GuardianAttackSelector(Guardian* guardian) : mGuardian(guardian)
{
}

Guardian::GuardianAttackSelector::~GuardianAttackSelector()
{
}

bool Guardian::GuardianAttackSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::SQUID, entity->GetType()))
           && entity->distanceToSqr(mGuardian->shared_from_this()) > 9.0;
}

Guardian::GuardianMoveControl::GuardianMoveControl(Guardian* guardian) : MoveControl(guardian), mGuardian(guardian)
{
}

Guardian::GuardianMoveControl::~GuardianMoveControl()
{
}

void Guardian::GuardianMoveControl::tick()
{
    if (mOperation != MOVE_TO || mGuardian->getNavigation()->isDone())
    {
        mGuardian->setSpeed(0.0f);
        mGuardian->setMoving(false);
        return;
    }

    double dx = mWantedX - mGuardian->mX;
    double dy = mWantedY - mGuardian->mY;
    double dz = mWantedZ - mGuardian->mZ;
    double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    dy /= distance;
    float yRot = (float)(std::atan2(dz, dx) * Mth::RAD_TO_DEG) - 90.0f;
    mGuardian->mYRot = rotlerp(mGuardian->mYRot, yRot, 30.0f);
    mGuardian->mYBodyRot = mGuardian->mYRot;

    double attributeSpeed = mGuardian->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue();
    float targetSpeed = (float)(mSpeedModifier * attributeSpeed);
    mGuardian->setSpeed(mGuardian->getSpeed() + (targetSpeed - mGuardian->getSpeed()) * 0.125f);

    double wag1 = std::sin((double)(mGuardian->mTickCount + mGuardian->getId()) * 0.5) * 0.05;
    double cosYRot = cosf((float)(mGuardian->mYRot * Mth::RAD_TO_DEG));
    double sinYRot = sinf((float)(mGuardian->mYRot * Mth::RAD_TO_DEG));
    mGuardian->mXd += wag1 * cosYRot;
    mGuardian->mZd += wag1 * sinYRot;
    double wag2 = std::sin((double)(mGuardian->mTickCount + mGuardian->getId()) * 0.75) * 0.05;
    mGuardian->mYd += wag2 * (sinYRot + cosYRot) * 0.25;
    mGuardian->mYd += mGuardian->getSpeed() * dy * 0.1;

    LookControl* lookControl = mGuardian->getLookControl();
    double lookX = mGuardian->mX + (dx / distance) * 2.0;
    double lookY = mGuardian->getEyeHeight() + mGuardian->mY + dy / distance;
    double lookZ = mGuardian->mZ + (dz / distance) * 2.0;
    double wantedX = lookControl->getWantedX();
    double wantedY = lookControl->getWantedY();
    double wantedZ = lookControl->getWantedZ();
    if (!lookControl->isHasWanted())
    {
        wantedX = lookX;
        wantedY = lookY;
        wantedZ = lookZ;
    }
    mGuardian->getLookControl()->setLookAt((lookX - wantedX) * 0.125 + wantedX, (lookY - wantedY) * 0.125 + wantedY,
                                           (lookZ - wantedZ) * 0.125 + wantedZ, 10.0f, 40.0f);
    mGuardian->setMoving(true);
}
