#include "game/core/Chicken.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

arrayWithLength<Item*> Chicken::FOOD_ITEMS;

Chicken::Chicken(Level* level) : Animal(level)
{
    fjDerivedCtorCalls();
    _init();
    setSize(0.4f, 0.7f);
    mTimeUntilNextEgg = mRandom->nextInt(6000) + 6000;
}

void Chicken::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::CHICKEN);
}

void Chicken::_init()
{
    mOFlapSpeed = 0.0f;
    mOFlap = 0.0f;
    mFlapTime = 0.0f;
    mFlapSpeed = 0.0f;
    mFlapping = 1.0f;
    mTimeUntilNextEgg = 0;
    setPathfindingMalus(BlockPathTypes::WATER, 0.0f);
}

void Chicken::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new PanicGoal(this, 1.4), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, new TemptGoal(this, 1.0, FOOD_ITEMS, false, arrayWithLength<int>()), true);
    mGoalSelector.addGoal(4, new FollowParentGoal(this, 1.1), true);
    mGoalSelector.addGoal(5, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(7, new RandomLookAroundGoal(this), true);
}

void Chicken::staticCtor()
{
    static Item* foodItems[] = {Items::wheatSeeds, Items::melonSeeds, Items::pumpkinSeeds, Items::beetrootSeeds};
    FOOD_ITEMS = arrayWithLength<Item*>(foodItems, 4);
}

float Chicken::getEyeHeight()
{
    return static_cast<double>(mBbHeight);
}

void Chicken::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(4.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
}

void Chicken::aiStep()
{
    Animal::aiStep();
    mOFlap = mFlapTime;
    mOFlapSpeed = mFlapSpeed;
    mFlapSpeed += (mOnGround ? -1.0f : 4.0f) * 0.3f;
    mFlapSpeed = Mth::clamp(mFlapSpeed, 0.0f, 1.0f);
    if (!mOnGround && mFlapping < 1.0f)
        mFlapping = 1.0f;
    mFlapping *= 0.9f;
    if (!mOnGround && mYd < 0.0)
        mYd *= 0.6;
    mFlapTime += mFlapping * 2.0f;
    if (!mLevel->mIsClientSide && !isBaby() && !canBeControlledByRider() && --mTimeUntilNextEgg < 1)
    {
        playSound(SoundEvents::CHICKEN_EGG, 1.0f, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
        spawnAtLocation(Items::egg, 1);
        mTimeUntilNextEgg = mRandom->nextInt(6000) + 6000;
    }
}

void Chicken::causeFallDamage(float, float)
{
}

const SoundEvent* Chicken::getAmbientSound()
{
    return SoundEvents::CHICKEN_AMBIENT;
}

const SoundEvent* Chicken::getHurtSound(const DamageSource*)
{
    return SoundEvents::CHICKEN_HURT;
}

const SoundEvent* Chicken::getDeathSound()
{
    return SoundEvents::CHICKEN_DEATH;
}

void Chicken::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::CHICKEN_STEP, 0.15f, 1.0f);
}

ResourceLocation* Chicken::getDefaultLootTable()
{
    return &BuiltInLootTables::CHICKEN;
}

boost::shared_ptr<AgableMob> Chicken::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<Chicken>(new Chicken(mLevel));
}

bool Chicken::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return std::find(FOOD_ITEMS.data, FOOD_ITEMS.data + FOOD_ITEMS.length, itemInstance->getItem())
           != FOOD_ITEMS.data + FOOD_ITEMS.length;
}

void Chicken::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    if (tag->contains(L"EggLayTime"))
        mTimeUntilNextEgg = tag->getInt(L"EggLayTime");
}

bool Chicken::canBeControlledByRider()
{
    return Entity::instanceof(getControllingPassenger(), EntityType::ZOMBIE)
           && boost::static_pointer_cast<Zombie>(getControllingPassenger())->isJockey();
}

void Chicken::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putInt(L"EggLayTime", mTimeUntilNextEgg);
}

bool Chicken::removeWhenFarAway()
{
    if (canBeControlledByRider())
        return !isVehicle() && Animal::removeWhenFarAway();
    return Animal::removeWhenFarAway();
}

void Chicken::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!RiderPositionOverrides::OverridePosition(shared_from_this(), passenger))
    {
        Entity::positionRider(passenger);
        float sinAngle = Mth::sin(mYBodyRot * Mth::DEG_TO_RAD);
        float cosAngle = Mth::cos(mYBodyRot * Mth::DEG_TO_RAD);
        double yOffset = 0.0;
        passenger->setPos(mX + static_cast<double>(0.1f * sinAngle),
                          mY + static_cast<double>(mBbHeight * 0.5f) + passenger->getRidingHeight() + yOffset,
                          mZ - static_cast<double>(0.1f * cosAngle));
    }
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, passenger->GetType()))
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(passenger);
        living->mYBodyRot = mYBodyRot;
    }
}

unsigned long long Chicken::GetType()
{
    return EntityType::CHICKEN;
}

Entity* Chicken::create(Level* level)
{
    return new Chicken(level);
}
