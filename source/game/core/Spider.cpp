#include "game/core/Spider.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Stray.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include <climits>
#include "game/entity/mob/Skeleton.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/ai/WallClimberNavigation.hpp"
#include "game/util/Random.hpp"

static const EntityDataAccessor<unsigned char>* SPIDER_DATA_FLAGS = NULL;

Spider::Spider(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    setSize(1.4f, 0.9f);
    mCategories |= EntityCategory::eEntityCategory_Arthropod;
}

Spider::Spider(Level* level, bool callDerivedConstructor) : Monster(level)
{
    if (callDerivedConstructor)
        fjDerivedCtorCalls();
    setSize(1.4f, 0.9f);
    mCategories |= EntityCategory::eEntityCategory_Arthropod;
}

Entity* Spider::create(Level* level)
{
    return new Spider(level);
}

void Spider::staticCtor()
{
    SPIDER_DATA_FLAGS = SynchedEntityData::defineId(EntityType::SPIDER, EntityType::MONSTER,
                                                    EntityDataSerializers::BYTE);
}

void Spider::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::SPIDER);
}

void Spider::registerGoals()
{
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(3, new LeapAtTargetGoal(this, 0.4f), true);
    mGoalSelector.addGoal(4, new SpiderAttackGoal(this), true);
    mGoalSelector.addGoal(5, new WaterAvoidingRandomStrollGoal(this, 0.8), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(6, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new SpiderTargetGoal(this, typeid(Player)), true);
    mTargetSelector.addGoal(3, new SpiderTargetGoal(this, typeid(VillagerGolem)), true);
}

double Spider::getRideHeight()
{
    return (double)(mBbHeight * 0.5f);
}

PathNavigation* Spider::createNavigation(Level* level)
{
    return new WallClimberNavigation(this, level);
}

void Spider::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(SPIDER_DATA_FLAGS, (unsigned char)0);
}

void Spider::tick()
{
    PIXBeginNamedEvent(0.0f, "Spider tick");
    Monster::tick();
    if (!mLevel->mIsClientSide)
        setClimbing(mHorizontalCollision);
    PIXEndNamedEvent();
}

void Spider::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(16.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.30000001192092896);
}

const SoundEvent* Spider::getAmbientSound()
{
    return SoundEvents::SPIDER_AMBIENT;
}
const SoundEvent* Spider::getHurtSound(const DamageSource*)
{
    return SoundEvents::SPIDER_HURT;
}
const SoundEvent* Spider::getDeathSound()
{
    return SoundEvents::SPIDER_DEATH;
}

void Spider::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::SPIDER_STEP, 0.15f, 1.0f);
}

ResourceLocation* Spider::getDefaultLootTable()
{
    return &BuiltInLootTables::SPIDER;
}
bool Spider::onLadder()
{
    return isClimbing();
}
void Spider::makeStuckInWeb()
{
}
EMobType Spider::getMobType()
{
    return MOB_TYPE_ARTHROPOD;
}

bool Spider::canBeAffected(MobEffectInstance* effect)
{
    if (effect->getEffect() == MobEffects::POISON)
        return false;
    return LivingEntity::canBeAffected(effect);
}

bool Spider::isClimbing()
{
    return (mEntityData->get(SPIDER_DATA_FLAGS) & 1) != 0;
}

void Spider::setClimbing(bool climbing)
{
    unsigned char flags = mEntityData->get(SPIDER_DATA_FLAGS);
    flags = climbing ? (unsigned char)(flags | 1) : (unsigned char)(flags & 0xfe);
    mEntityData->set(SPIDER_DATA_FLAGS, flags, true);
}

MobGroupData* Spider::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, const std::wstring&)
{
    data = Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));

    if (mLevel->mRandom->nextInt(100) == 0)
    {
        boost::shared_ptr<AbstractSkeleton> rider;

        if (mLevel->canSeeSky(BlockPos(mX, mY, mZ)) && !mLevel->isDay() && mLevel->mRandom->rollPercentageChance(80)
            && (mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::ICE_PLAINS
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::ICE_MOUNTAINS
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::ICE_PLAINS_SPIKES
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::FROZEN_RIVER
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::COLD_BEACH
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::FROZEN_OCEAN
                || mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::DEEP_FROZEN_OCEAN))
        {
            rider = boost::shared_ptr<Stray>(new Stray(mLevel));
        }
        else if (mLevel->mRandom->rollPercentageChance(80) && mLevel->getBiome(BlockPos(mX, mY, mZ)) == Biome::HELL)
        {
            rider = boost::shared_ptr<WitherSkeleton>(new WitherSkeleton(mLevel));
        }
        else
        {
            rider = boost::shared_ptr<Skeleton>(new Skeleton(mLevel));
        }

        rider->moveTo(mX, mY, mZ, mYRot, 0.0f);
        rider->finalizeMobSpawn(difficulty, NULL, L"");
        mLevel->addEntity(rider);
        rider->startRiding(shared_from_this());
    }

    if (data == NULL)
    {
        data = new SpiderEffectsGroupData();
        if (mLevel->getDifficulty() == Difficulty::HARD
            && mLevel->mRandom->nextFloat() < 0.1f * difficulty.getSpecialMultiplier())
        {
            static_cast<SpiderEffectsGroupData*>(data)->setRandomEffect(mLevel->mRandom);
        }
    }

    if (dynamic_cast<SpiderEffectsGroupData*>(data) != NULL)
    {
        MobEffect* effect = static_cast<SpiderEffectsGroupData*>(data)->mEffect;
        if (effect != NULL)
            addEffect(new MobEffectInstance(effect, INT_MAX), boost::shared_ptr<Entity>());
    }

    return data;
}

float Spider::getEyeHeight()
{
    return 0.65f;
}
unsigned long long Spider::GetType()
{
    return EntityType::SPIDER;
}

Spider::SpiderAttackGoal::SpiderAttackGoal(Spider* spider) : MeleeAttackGoal(spider, 1.0, true)
{
}

bool Spider::SpiderAttackGoal::canContinueToUse()
{
    float brightness = mMob->getBrightness();
    if (brightness >= 0.5f && mMob->getRandom()->nextInt(100) == 0)
    {
        mMob->setTarget(boost::shared_ptr<LivingEntity>());
        return false;
    }
    return MeleeAttackGoal::canContinueToUse();
}

double Spider::SpiderAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    return (double)(target->mBbWidth + 4.0f);
}

Spider::SpiderEffectsGroupData::SpiderEffectsGroupData() : mEffect(NULL)
{
}
Spider::SpiderEffectsGroupData::~SpiderEffectsGroupData()
{
}

void Spider::SpiderEffectsGroupData::setRandomEffect(Random* random)
{
    int roll = random->nextInt(5);
    if (roll <= 1)
        mEffect = MobEffects::MOVEMENT_SPEED;
    else if (roll <= 2)
        mEffect = MobEffects::DAMAGE_BOOST;
    else if (roll <= 3)
        mEffect = MobEffects::REGENERATION;
    else if (roll <= 4)
        mEffect = MobEffects::INVISIBILITY;
}

Spider::SpiderTargetGoal::SpiderTargetGoal(Spider* spider, const std::type_info& targetType)
    : NearestAttackableTargetGoal(spider, targetType, true, false, false)
{
}

bool Spider::SpiderTargetGoal::canUse()
{
    float brightness = mMob->getBrightness();
    if (brightness < 0.5f)
    {
        return NearestAttackableTargetGoal::canUse();
    }
    return false;
}
