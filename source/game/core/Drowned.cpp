#include "game/core/Drowned.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/EquipmentSlot.hpp"

#include <cmath>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/DrownedPathNavigation.hpp"
#include "game/entity/ai/FleeSunGoal.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/ai/RestrictSunGoal.hpp"
#include "game/entity/ai/SwimMoveControl.hpp"
#include "game/entity/ai/UnderwaterOrInDarknessSelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/RemoveBlockGoal.hpp"
#include "game/world/generation/MoveThroughVillageGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/core/PIXNamedEvents.hpp"

static Predicate<boost::shared_ptr<Entity> >* DROWNED_TARGET_SELECTOR = new UnderwaterOrInDarknessSelector();

Drowned::Drowned(Level* level) : Zombie(level, false)
{
    mMeleeAttackGoal = NULL;
    mRangedAttackGoal = NULL;
    mIsRangedAttacking = false;
    setSwimmer(true);
    fjDerivedCtorCalls();
    delete mMoveControl;
    mMoveControl = new SwimMoveControl(this);
}

Drowned::~Drowned()
{
    delete mMeleeAttackGoal;
    delete mRangedAttackGoal;
}

Entity* Drowned::create(Level* level)
{
    return new Drowned(level);
}

void Drowned::performRangedAttack(boost::shared_ptr<LivingEntity> target, float)
{
    boost::shared_ptr<ThrownTrident> trident(new ThrownTrident(mLevel, mX, mY + getEyeHeight(), mZ));
    trident->mOwner = shared_from_this();
    double dx = target->mX - mX;
    double dy = target->mBoundingBox->mMinY + target->mBbHeight / 3.0f - trident->mY;
    double dz = target->mZ - mZ;
    double distanceSqr = dx * dx + dz * dz;
    double distance = std::sqrt(distanceSqr);
    trident->shoot(dx, dy + distance * 0.2f, dz, 1.6f, 14.0f - (float)mLevel->getDifficulty()->getId() * 4.0f);
    playSound(SoundEvents::TRIDENT_THROW, 1.0f, 1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
    mLevel->addEntity(trident);
}

bool Drowned::isAggressive()
{
    return Zombie::isAggressive();
}

void Drowned::setAggressive(bool aggressive)
{
    if (getMainHandItem()->getItem() == Items::trident)
    {
        if (aggressive)
            startUsingItem(InteractionHand::eInteractionHand_MainHand);
        else
            stopUsingItem();
    }
    Zombie::setAggressive(aggressive);
}

bool Drowned::breathesWater()
{
    return true;
}

bool Drowned::checkSpawnRules()
{
    return mLevel->getDifficulty() != Difficulty::PEACEFUL && !mLevel->containsAnysolidBlocking(mBoundingBox)
           && getWalkTargetValue(BlockPos(mX, mBoundingBox->mMinY, mZ)) >= 0.0f;
}

bool Drowned::checkSpawnObstruction()
{
    return mLevel->isUnobstructed(mBoundingBox, shared_from_this())
           && mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty();
}

void Drowned::aiStep()
{
    Zombie::aiStep();
    if (mLevel->mIsClientSide)
        return;

    boost::shared_ptr<LivingEntity> target = getTarget();
    if (target != NULL)
    {
        setAggressive(true);

        if (mRangedAttackGoal != NULL)
        {
            double distance = target->distanceToSqr(shared_from_this());
            if (mIsRangedAttacking || distance >= 25.0)
            {
                if (!mIsRangedAttacking)
                {
                    mGoalSelector.removeGoal(mMeleeAttackGoal);
                    mGoalSelector.addGoal(3, mRangedAttackGoal, false);
                    mIsRangedAttacking = true;
                }
                if (distance <= 9.0)
                {
                    mRangedAttackGoal->stop();
                    mGoalSelector.removeGoal(mRangedAttackGoal);
                    mGoalSelector.addGoal(3, mMeleeAttackGoal, false);
                    mIsRangedAttacking = false;
                }
            }
        }
        else
        {
            if (getMainHandItem()->getItem() == Items::trident)
                mRangedAttackGoal = new RangedAttackGoal(this, this, 1.0, 20, 60, 10.0f);
        }
    }
    else
    {
        if (mRangedAttackGoal != NULL)
        {
            if (!mRangedAttackGoal->canContinueToUse())
                setAggressive(false);
        }
        else
        {
            setAggressive(false);
        }
    }
}

void Drowned::registerAttributes()
{
    Zombie::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23f);
}

void Drowned::registerGoals()
{
    mMeleeAttackGoal = new MeleeAttackGoal(this, 1.0, false);
    mGoalSelector.addGoal(2, new RestrictSunGoal(this), true);
    mGoalSelector.addGoal(2, new FleeSunGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, mMeleeAttackGoal, false);
    mGoalSelector.addGoal(4,
                          new RemoveBlockGoal(Blocks::turtle_egg, this, 1.0, 16, 2, 1.6, SoundEvents::TURTLE_EGG_HATCH,
                                              SoundEvents::TURTLE_EGG_BREAK),
                          true);
    mGoalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new RandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mGoalSelector.addGoal(6, new MoveThroughVillageGoal(this, 1.0, false), true);
    mTargetSelector.addGoal(
        2,
        new NearestAttackableTargetGoal(this, typeid(Player), 10, true, false, DROWNED_TARGET_SELECTOR, false, false),
        true);
    mTargetSelector.addGoal(2,
                            new NearestAttackableTargetGoal(this, typeid(Villager), 10, false, false,
                                                            DROWNED_TARGET_SELECTOR, false, false),
                            true);
    mTargetSelector.addGoal(2,
                            new NearestAttackableTargetGoal(this, typeid(VillagerGolem), 10, true, false,
                                                            DROWNED_TARGET_SELECTOR, false, false),
                            true);
    mTargetSelector.addGoal(3,
                            new NearestAttackableTargetGoal(this, typeid(Turtle), 10, true, false,
                                                            Turtle::BABY_ON_LAND_SELECTOR, false, false),
                            true);
}

const SoundEvent* Drowned::getAmbientSound()
{
    return isInWater() ? SoundEvents::DROWNED_AMBIENT_WATER : SoundEvents::DROWNED_AMBIENT;
}

const SoundEvent* Drowned::getHurtSound(const DamageSource*)
{
    return isInWater() ? SoundEvents::DROWNED_HURT_WATER : SoundEvents::DROWNED_HURT;
}

const SoundEvent* Drowned::getDeathSound()
{
    return isInWater() ? SoundEvents::DROWNED_DEATH_WATER : SoundEvents::DROWNED_DEATH;
}

const SoundEvent* Drowned::getStepSound()
{
    return SoundEvents::DROWNED_STEP;
}

float Drowned::getWaterSlowDown()
{
    return 0.9f;
}

void Drowned::travel(float xxa, float yya, float zza)
{
    boost::shared_ptr<LivingEntity> target = getTarget();
    if (isEffectiveAi() && target == NULL)
    {
        stopSwimming();
    }

    if (isInWaterOrBubble() && isEffectiveAi())
    {
        PIXBeginNamedEvent(0.0f, "Travel in water");
        double oldY = mY;
        float slowDown = getWaterSlowDown();
        double gravity = 0.0;
        moveRelative(xxa, yya, zza, 0.06f);
        move(eMoverType_Self, mXd, mYd, mZd, false);
        mXd *= slowDown;
        mZd *= slowDown;

        if (target != NULL)
        {
            if (target->mOnGround && !mOnGround)
            {
                gravity = 0.005;
            }
        }
        else if (shouldApplyWaterGravity() && !isSwimming())
        {
            gravity = 0.02f;
            slowDown = LivingEntity::getWaterSlowDown();
        }

        mYd = mYd * slowDown - gravity;
        if (mHorizontalCollision && isFree(mXd, mYd + 0.6f - mY + oldY, mZd))
        {
            mYd = 0.3f;
        }
        PIXEndNamedEvent();
        updateModelAnimation();
    }
    else
    {
        LivingEntity::travel(xxa, yya, zza);
    }
}

not_null_ptr<ItemInstance> Drowned::getSkull()
{
    return ItemInstance::EMPTY;
}

ResourceLocation* Drowned::getDefaultLootTable()
{
    return &BuiltInLootTables::DROWNED;
}

ResourceLocation* Drowned::getDefaultGearTable()
{
    return mRandom->nextFloat() < 0.85f ? &BuiltInLootTables::DROWNED_EQUIPMENT
                                        : &BuiltInLootTables::DROWNED_RANGED_EQUIPMENT;
}

void Drowned::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    setLeftHanded(false);
    Mob::populateDefaultEquipmentSlots(difficulty);
    setDropChance(EquipmentSlot::OFFHAND, 1.0f);
}

PathNavigation* Drowned::createNavigation(Level* level)
{
    return new DrownedPathNavigation(this, level);
}

unsigned long long Drowned::GetType()
{
    return EntityType::DROWNED;
}
