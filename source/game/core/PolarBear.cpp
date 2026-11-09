#include "game/core/PolarBear.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/Mth.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"


const EntityDataAccessor<bool>* PolarBear::DATA_STANDING = NULL;

PolarBear::PolarBear(Level* level) : Animal(level)
{
    fjDerivedCtorCalls();
    setSize(1.3f, 1.4f);
    mStandingScale = 0.0f;
    mWarningSoundTicks = 0;
    mStandingScaleO = 0.0f;
}

Entity* PolarBear::create(Level* level)
{
    return new PolarBear(level);
}

void PolarBear::staticCtor()
{
    DATA_STANDING = SynchedEntityData::defineId<bool>(EntityType::POLAR_BEAR, EntityType::ANIMAL,
                                                      EntityDataSerializers::BOOLEAN);
}

boost::shared_ptr<AgableMob> PolarBear::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<PolarBear>(new PolarBear(mLevel));
}

bool PolarBear::isFood(const not_null_ptr<ItemInstance>&)
{
    return false;
}

void PolarBear::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new PolarBearMeleeAttackGoal(this), true);
    mGoalSelector.addGoal(1, new PolarBearPanicGoal(this), true);
    mGoalSelector.addGoal(4, new FollowParentGoal(this, 1.25), true);
    mGoalSelector.addGoal(5, new RandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(7, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new PolarBearHurtByTargetGoal(this), true);
    mTargetSelector.addGoal(2, new PolarBearAttackPlayersGoal(this), true);
}

void PolarBear::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(20.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(6.0);
}

const SoundEvent* PolarBear::getAmbientSound()
{
    return isBaby() ? SoundEvents::POLARBEAR_BABY_AMBIENT : SoundEvents::POLARBEAR_AMBIENT;
}

const SoundEvent* PolarBear::getHurtSound(const DamageSource*)
{
    return SoundEvents::POLARBEAR_HURT;
}

const SoundEvent* PolarBear::getDeathSound()
{
    return SoundEvents::POLARBEAR_DEATH;
}

void PolarBear::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::POLARBEAR_STEP, 0.15f, 1.0f);
}

void PolarBear::playWarningSound()
{
    if (mWarningSoundTicks < 1)
    {
        playSound(SoundEvents::POLARBEAR_WARNING, 1.0f, 1.0f);
        mWarningSoundTicks = 40;
    }
}

ResourceLocation* PolarBear::getDefaultLootTable()
{
    return &BuiltInLootTables::POLAR_BEAR;
}

void PolarBear::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->createDateItem(DATA_STANDING, false);
}

bool PolarBear::isStanding()
{
    return mEntityData->get(DATA_STANDING);
}

void PolarBear::tick()
{
    Mob::tick();
    if (mLevel->mIsClientSide)
    {
        mStandingScaleO = mStandingScale;
        if (isStanding())
            mStandingScale = Mth::clamp(mStandingScale + 1.0f, 0.0f, 6.0f);
        else
            mStandingScale = Mth::clamp(mStandingScale - 1.0f, 0.0f, 6.0f);
    }
    if (mWarningSoundTicks > 0)
        --mWarningSoundTicks;
}

bool PolarBear::doHurtTarget(boost::shared_ptr<Entity> target)
{
    EntityDamageSource* source = DamageSource::CreateMobAttack(
        boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
    bool hurt = target->hurt(source,
                             static_cast<float>(getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue()));
    delete source;
    if (hurt)
        doEnchantDamageEffects(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()), target);
    return hurt;
}

void PolarBear::setStanding(bool standing)
{
    mEntityData->set(DATA_STANDING, standing, true);
}

float PolarBear::getStandingAnimationScale(float partialTick)
{
    return ((mStandingScale - mStandingScaleO) * partialTick + mStandingScaleO) / 6.0f;
}

float PolarBear::getWaterSlowDown()
{
    return 0.98;
}

MobGroupData* PolarBear::finalizeMobSpawn(const DifficultyInstance&, MobGroupData* data, const std::wstring&)
{
    if (data != NULL && dynamic_cast<PolarBearGroupData*>(data) != NULL)
    {
        if (static_cast<PolarBearGroupData*>(data)->mIsBaby)
        {
            setAge(-24000);
        }
    }
    else
    {
        PolarBearGroupData* group = new PolarBearGroupData();
        data = group;
        group->mIsBaby = true;
    }
    return data;
}

bool PolarBear::checkSpawnRules()
{
    int x = Mth::floor(mX);
    int y = Mth::floor(mBoundingBox->mMinY);
    int z = Mth::floor(mZ);
    BlockPos pos(x, y, z);
    Biome* biome = mLevel->getBiome(pos);
    if (biome == Biome::FROZEN_OCEAN || biome == Biome::DEEP_FROZEN_OCEAN)
    {
        return mLevel->getRawBrightness(pos, false) > 8
               && mLevel->getBlockState(pos.below())->getBlock() == Blocks::ice;
    }
    return Animal::checkSpawnRules();
}

unsigned long long PolarBear::GetType()
{
    return EntityType::POLAR_BEAR;
}

PolarBear::PolarBearGroupData::PolarBearGroupData() : mIsBaby(false)
{
}

PolarBear::PolarBearGroupData::~PolarBearGroupData()
{
}

PolarBear::PolarBearAttackPlayersGoal::PolarBearAttackPlayersGoal(PolarBear* polarBear)
    : NearestAttackableTargetGoal(polarBear, typeid(Player), 20, true, true, NULL, false, true)
{
}

bool PolarBear::PolarBearAttackPlayersGoal::canUse()
{
    if (mMob->mLevel->getDifficulty() != Difficulty::PEACEFUL && !mMob->isBaby()
        && NearestAttackableTargetGoal::canUse())
    {
        std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(
            typeid(PolarBear), mMob->mBoundingBox->grow(8.0, 4.0, 8.0));

        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<PolarBear> bear = boost::static_pointer_cast<PolarBear>(*it);
            if (bear->isBaby())
            {
                delete entities;
                return true;
            }
        }

        delete entities;
        mMob->setTarget(boost::shared_ptr<LivingEntity>());
    }
    return false;
}

double PolarBear::PolarBearAttackPlayersGoal::getFollowDistance()
{
    return TargetGoal::getFollowDistance() * 0.5;
}

PolarBear::PolarBearHurtByTargetGoal::PolarBearHurtByTargetGoal(PolarBear* polarBear)
    : HurtByTargetGoal(polarBear, false, arrayWithLength<unsigned long long>())
{
}

void PolarBear::PolarBearHurtByTargetGoal::start()
{
    HurtByTargetGoal::start();
    if (mMob->isBaby())
    {
        alertOthers();
        stop();
    }
}

void PolarBear::PolarBearHurtByTargetGoal::alertOther(boost::shared_ptr<PathfinderMob> mob,
                                                      boost::shared_ptr<LivingEntity> target)
{
    bool isPolarBear = eTYPE_DERIVED_FROM(EntityType::POLAR_BEAR, mob->GetType());
    if (isPolarBear)
    {
        if (!boost::static_pointer_cast<PolarBear>(mob)->isBaby())
        {
            HurtByTargetGoal::alertOther(mob, target);
        }
    }
}

PolarBear::PolarBearMeleeAttackGoal::PolarBearMeleeAttackGoal(PolarBear* polarBear)
    : MeleeAttackGoal(polarBear, 1.25, true)
{
}

bool PolarBear::PolarBearMeleeAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL)
    {
        if (target)
        {
            bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType());
            if (isPlayer)
                return false;
        }
    }
    return MeleeAttackGoal::canUse();
}

void PolarBear::PolarBearMeleeAttackGoal::checkAndPerformAttack(boost::shared_ptr<LivingEntity> target, double distance)
{
    double reach = getAttackReachSqr(target);
    if (distance <= reach && mTicksUntilNextAttack < 1)
    {
        mTicksUntilNextAttack = mAttackInterval;
        mMob->doHurtTarget(target);
        static_cast<PolarBear*>(mMob)->setStanding(false);
    }
    else if (distance <= reach + reach)
    {
        if (mTicksUntilNextAttack < 1)
        {
            static_cast<PolarBear*>(mMob)->setStanding(false);
            mTicksUntilNextAttack = mAttackInterval;
        }
        if (mTicksUntilNextAttack <= 10)
        {
            static_cast<PolarBear*>(mMob)->setStanding(true);
            static_cast<PolarBear*>(mMob)->playWarningSound();
        }
    }
    else
    {
        mTicksUntilNextAttack = mAttackInterval;
        static_cast<PolarBear*>(mMob)->setStanding(false);
    }
}

void PolarBear::PolarBearMeleeAttackGoal::stop()
{
    static_cast<PolarBear*>(mMob)->setStanding(false);
    MeleeAttackGoal::stop();
}

double PolarBear::PolarBearMeleeAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    return target->mBbWidth + 4.0f;
}

PolarBear::PolarBearPanicGoal::PolarBearPanicGoal(PolarBear* polarBear) : PanicGoal(polarBear, 2.0)
{
}

PolarBear::PolarBearPanicGoal::~PolarBearPanicGoal()
{
}

bool PolarBear::PolarBearPanicGoal::canUse()
{
    if (!mMob->isBaby() && !mMob->isOnFire())
    {
        return false;
    }
    return PanicGoal::canUse();
}
