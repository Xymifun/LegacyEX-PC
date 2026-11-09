#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

namespace
{
}

#include <cmath>
#include <typeinfo>
#include <set>
#include <vector>

#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/ParentHorseSelector.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RunAroundLikeCrazyGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/HorseChest.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/RangedAttribute.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"

namespace
{
const Predicate<boost::shared_ptr<Entity> >* PARENT_HORSE_SELECTOR = new ParentHorseSelector();
}

const EntityDataAccessor<unsigned char>* AbstractHorse::DATA_FLAGS = NULL;
const EntityDataAccessor<std::wstring>* AbstractHorse::DATA_OWNER_UUID = NULL;
Attribute* AbstractHorse::JUMP_STRENGTH =
    (new RangedAttribute(NULL, eATTRIBUTE_ID_HorseJumpStrength, 0.7, 0.0, 2.0))->setSyncable(true);

AbstractHorse::AbstractHorse(Level* level) : Animal(level)
{
    mPanicGoal = 0;
    mFloatGoal = 0;
    mWaterAvoidingRandomStrollGoal = 0;
    setSize(1.3964844f, 1.6f);
    mEatingCounter = 0;
    mMouthCounter = 0;
    mStandCounter = 0;
    mTailCounter = 0;
    mSprintCounter = 0;
    mLastAngrySoundTime = 0;
    mJumping = false;
    mTemper = 0;
    mJumpRidingScale = 0.0f;
    mAllowStandSliding = false;
    mEatAnim = 0.0f;
    mEatAnimO = 0.0f;
    mStandAnim = 0.0f;
    mStandAnimO = 0.0f;
    mMouthAnim = 0.0f;
    mMouthAnimO = 0.0f;
    mCanGallop = true;
    mGallopSoundCounter = 0;
    mMaxUpStep = 1.0f;
}

bool AbstractHorse::getFlag(int flag)
{
    return (mEntityData->get(DATA_FLAGS) & flag) != 0;
}
bool AbstractHorse::isSaddled()
{
    return getFlag(4);
}

void AbstractHorse::containerChanged(Container*)
{
    const bool saddled = isSaddled();
    updateEquipment();
    if (mTickCount > 20 && !saddled && isSaddled())
        playSound(SoundEvents::HORSE_SADDLE, 0.5f, 1.0f);
}

void AbstractHorse::setFlag(int flag, bool value)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    if (value)
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags | flag), true);
    else
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags & ~flag), true);
}

void AbstractHorse::setEating(bool eating)
{
    setFlag(16, eating);
}

void AbstractHorse::setStanding(bool standing)
{
    if (standing)
        setEating(false);
    setFlag(32, standing);
}

void AbstractHorse::stand()
{
    if (mLevel->mIsClientSide)
        return;
    mStandCounter = 1;
    setStanding(true);
}

void AbstractHorse::onPlayerJump(int jump)
{
    if (!isSaddled() && !eTYPE_DERIVED_FROM(EntityType::SKELETON_HORSE, GetType()))
        return;
    if (jump < 0)
    {
        jump = 0;
    }
    else
    {
        mAllowStandSliding = true;
        stand();
    }
    if (jump >= 90)
    {
        mJumpRidingScale = 1.0f;
        return;
    }
    mJumpRidingScale = 0.4f * (float)jump / 90.0f + 0.4f;
}

bool AbstractHorse::canJump()
{
    return isSaddled() || eTYPE_DERIVED_FROM(EntityType::SKELETON_HORSE, GetType());
}
double AbstractHorse::getJumpRidingScale()
{
    return mJumpRidingScale;
}
void AbstractHorse::setJumpRidingScale(float scale)
{
    mJumpRidingScale = scale;
}

void AbstractHorse::handleStartJump(int)
{
    boost::shared_ptr<Entity> passenger = getControllingPassenger();
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, passenger->GetType()))
    {
        mLevel->playSound(boost::static_pointer_cast<Player>(passenger), BlockPos(mX, mY, mZ), getJumpSound(),
                          SoundSource::eSoundSource_Players, 0.4f, 1.0f, 16.0f);
    }
    else
    {
        mLevel->playSound(boost::shared_ptr<Player>(), BlockPos(mX, mY, mZ), getJumpSound(),
                          SoundSource::eSoundSource_Players, 0.4f, 1.0f, 16.0f);
    }
    mAllowStandSliding = true;
    stand();
}

void AbstractHorse::handleStopJump()
{
}

std::wstring AbstractHorse::getOwnerUUID()
{
    return mEntityData->get(DATA_OWNER_UUID);
}

boost::shared_ptr<Entity> AbstractHorse::getOwner()
{
    std::wstring uuid = getOwnerUUID();
    if (uuid.empty())
        return boost::shared_ptr<Entity>();
    return mLevel->getPlayerByUUID(uuid);
}

int AbstractHorse::getTailCounter() const
{
    return mTailCounter;
}

void AbstractHorse::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId(EntityType::ABSTRACT_HORSE, EntityType::ANIMAL,
                                             EntityDataSerializers::BYTE);
    DATA_OWNER_UUID = SynchedEntityData::defineId(EntityType::ABSTRACT_HORSE, EntityType::ANIMAL,
                                                  EntityDataSerializers::STRING);
}

void AbstractHorse::fjDerivedCtorCalls()
{
    Mob::fjDerivedCtorCalls();
    createInventory();
}

void AbstractHorse::registerGoals()
{
    mFloatGoal = new FloatGoal(this);
    mGoalSelector.addGoal(0, mFloatGoal, true);

    mPanicGoal = new PanicGoal(this, 1.2);
    mGoalSelector.addGoal(1, mPanicGoal, true);
    mGoalSelector.addGoal(1, new RunAroundLikeCrazyGoal(this, 1.2), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 1.0, typeid(AbstractHorse)), true);
    mGoalSelector.addGoal(4, new FollowParentGoal(this, 1.0), true);

    mWaterAvoidingRandomStrollGoal = new WaterAvoidingRandomStrollGoal(this, 0.7);
    mGoalSelector.addGoal(6, mWaterAvoidingRandomStrollGoal, true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
}

void AbstractHorse::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
    mEntityData->define(DATA_OWNER_UUID, std::wstring(L""));
}

bool AbstractHorse::isTamed()
{
    return getFlag(2);
}

void AbstractHorse::setOwnerUUID(const std::wstring& uuid)
{
    mEntityData->set(DATA_OWNER_UUID, uuid, true);
}

float AbstractHorse::getFoalScale() const
{
    return 0.5f;
}

void AbstractHorse::updateSize(bool baby)
{
    if (baby)
        internalSetSize(getFoalScale());
    else
        internalSetSize(1.0f);
}

bool AbstractHorse::isJumping() const
{
    return mJumping;
}
void AbstractHorse::setTamed(bool tamed)
{
    setFlag(2, tamed);
}
void AbstractHorse::setIsJumping(bool jumping)
{
    mJumping = jumping;
}

bool AbstractHorse::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return Mob::canBeLeashed(player);
}

bool AbstractHorse::isEating()
{
    return getFlag(16);
}

void AbstractHorse::onLeashDistance(float distance)
{
    if (distance > 6.0f && isEating())
        setEating(false);
}

bool AbstractHorse::isStanding()
{
    return getFlag(32);
}
bool AbstractHorse::isBred()
{
    return getFlag(8);
}
void AbstractHorse::setBred(bool bred)
{
    setFlag(8, bred);
}

void AbstractHorse::setSaddled(bool saddled)
{
    if (saddled)
        setDespawnProtected();
    setFlag(4, saddled);
}

int AbstractHorse::getTemper() const
{
    return mTemper;
}
void AbstractHorse::setTemper(int temper)
{
    mTemper = temper;
}

int AbstractHorse::modifyTemper(int amount)
{
    int temper = Mth::clamp(getTemper() + amount, 0, getMaxTemper());
    setTemper(temper);
    return temper;
}

bool AbstractHorse::hurt(const DamageSource* source, float damage)
{
    if (isTamed())
    {
        boost::shared_ptr<Entity> direct = source->getDirectEntity();
        if (direct && eTYPE_DERIVED_FROM(EntityType::PLAYER, direct->GetType()))
        {
            if (!boost::static_pointer_cast<Player>(direct)->canHarmPlayer(getOwnerUUID()))
                return false;
        }
    }

    boost::shared_ptr<Entity> entity = source->getEntity();
    bool blocked = false;
    if (isVehicle() && entity && hasIndirectPassenger(entity))
        blocked = true;
    if (blocked)
        return false;

    setEating(false);
    return Animal::hurt(source, damage);
}

bool AbstractHorse::isPushable(boost::shared_ptr<Entity> entity)
{
    return !isVehicle() || eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType());
}

void AbstractHorse::openMouth()
{
    if (mLevel->mIsClientSide)
        return;
    mMouthCounter = 1;
    setFlag(64, true);
}

void AbstractHorse::eating(int animation)
{
    openMouth();
    if (!isSilent())
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::HORSE_EAT, getSoundSource(), 1.0f,
                          (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f, 16.0f);
    }
    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::EAT_ITEM_PARTICLES, animation);
}

void AbstractHorse::causeFallDamage(float fallDistance, float damageMultiplier)
{
    if (fallDistance > 1.0f)
        playSound(SoundEvents::HORSE_LAND, 0.4f, 1.0f);

    const float scaled = (fallDistance * 0.5f - 3.0f) * damageMultiplier;
    int damage = static_cast<int>(scaled);
    if (static_cast<float>(damage) < scaled)
        ++damage;

    if (static_cast<float>(damage) <= 0.0f)
        return;

    hurt(DamageSource::FALL, static_cast<float>(damage));
    if (isVehicle())
    {
        std::set<boost::shared_ptr<Entity> >* passengers = getIndirectPassengers();
        for (std::set<boost::shared_ptr<Entity> >::iterator it = passengers->begin(); it != passengers->end(); ++it)
        {
            boost::shared_ptr<Entity> passenger = *it;
            passenger->hurt(DamageSource::FALL, static_cast<float>(damage));
        }
    }

    Block* block = mLevel->getBlockState(BlockPos(mX, mY - 0.2 - static_cast<double>(mYRotO), mZ))->getBlock();
    if (block->mMaterial == Material::AIR || isSilent())
        return;

    const SoundType* soundType = block->mSoundType;
    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, soundType->getStepSound(), getSoundSource(),
                      soundType->getVolume() * 0.5f, soundType->getPitch() * 0.75f, 16.0f);
}

bool AbstractHorse::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    bool nonEmpty = !item->isEmpty();
    if (nonEmpty && handleEating(player, item))
    {
        if (!player->mAbilities.mInstabuild)
            item->shrink(1);
        return true;
    }
    return Animal::mobInteract(player, hand);
}

int AbstractHorse::getInventorySize()
{
    return 2;
}

void AbstractHorse::createInventory()
{
    boost::shared_ptr<SimpleContainer> previous = mInventory;
    mInventory = boost::shared_ptr<HorseChest>(new HorseChest(L"HorseChest", getInventorySize()));
    mInventory->setCustomName(getName());
    if (previous)
    {
        previous->removeListener(this);
        int count = previous->getContainerSize();
        const unsigned int size = mInventory->getContainerSize();
        if (size < count)
            count = size;
        for (int slot = 0; slot < count; ++slot)
        {
            not_null_ptr<ItemInstance> item = previous->getItem(slot);
            if (!item->isEmpty())
                mInventory->setItem(slot, item->copy());
        }
        previous.reset();
    }
    mInventory->addListener(this);
    updateEquipment();
}

void AbstractHorse::updateEquipment()
{
    if (mLevel->mIsClientSide)
        return;
    not_null_ptr<ItemInstance> saddle = mInventory->getItem(0);
    setSaddled(!saddle->isEmpty() && canBeSaddled());
}

boost::shared_ptr<AbstractHorse> AbstractHorse::getClosestMommy(const boost::shared_ptr<Entity>& entity,
                                                                double distance)
{
    double best = 1.7976931348623157e+308;
    boost::shared_ptr<Entity> closest;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        entity, entity->mBoundingBox->expand(distance, distance, distance), PARENT_HORSE_SELECTOR, NULL);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> candidate = *it;
        const double distanceSqr = candidate->distanceToSqr(entity->mX, entity->mY, entity->mZ);
        if (distanceSqr < best)
        {
            best = distanceSqr;
            closest = candidate;
        }
    }
#if !defined(RECON_NATIVE)
    delete entities;
#endif
    return boost::static_pointer_cast<AbstractHorse>(closest);
}

double AbstractHorse::getCustomJump()
{
    return getAttribute(JUMP_STRENGTH)->getValue();
}

const SoundEvent* AbstractHorse::getJumpSound()
{
    return SoundEvents::HORSE_JUMP;
}
const SoundEvent* AbstractHorse::getDeathSound()
{
    return NULL;
}
const SoundEvent* AbstractHorse::getHurtSound(const DamageSource*)
{
    return NULL;
}
const SoundEvent* AbstractHorse::getAmbientSound()
{
    return NULL;
}
const SoundEvent* AbstractHorse::getAngrySound()
{
    return NULL;
}
bool AbstractHorse::canBeSaddled() const
{
    return true;
}

void AbstractHorse::playStepSound(const BlockPos& pos, Block* block)
{
    const SoundType* soundType = block->getSoundType();
    if (block->mMaterial->isLiquid())
        return;

    Block* above = mLevel->getBlockState(pos.above())->getBlock();
    if (above == Blocks::snow_layer)
        soundType = Blocks::snow_layer->getSoundType();

    if (isVehicle() && mCanGallop)
    {
        ++mGallopSoundCounter;
        if (mGallopSoundCounter > 5 && mGallopSoundCounter % 3 == 0)
            playGallopSound(soundType);
        else if (mGallopSoundCounter <= 5)
            playSound(SoundEvents::HORSE_STEP_WOOD, soundType->getVolume() * 0.4f, soundType->getPitch());
        return;
    }

    if (soundType == SoundType::WOOD)
        playSound(SoundEvents::HORSE_STEP_WOOD, soundType->getVolume() * 0.4f, soundType->getPitch());
    else
        playSound(SoundEvents::HORSE_STEP, soundType->getVolume() * 0.4f, soundType->getPitch());
}

void AbstractHorse::playGallopSound(const SoundType* sound)
{
    playSound(SoundEvents::HORSE_GALLOP, sound->getVolume() * 0.4f, sound->getPitch());
}

void AbstractHorse::registerAttributes()
{
    Mob::registerAttributes();
    getAttributes()->registerAttribute(JUMP_STRENGTH);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(53.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.22499999403953552);
}

int AbstractHorse::getMaxSpawnClusterSize()
{
    return 6;
}
int AbstractHorse::getMaxTemper() const
{
    return 100;
}
float AbstractHorse::getSoundVolume()
{
    return 0.8f;
}
int AbstractHorse::getAmbientSoundInterval()
{
    return 400;
}

void AbstractHorse::openInventory(const boost::shared_ptr<Player>& player)
{
    if (!mLevel->mIsClientSide && (!isVehicle() || hasPassenger(player)) && isTamed())
    {
        mInventory->setCustomName(getName());
        player->openHorseInventory(boost::static_pointer_cast<AbstractHorse>(shared_from_this()), mInventory);
    }
}

int AbstractHorse::CheckAndSetInLove(boost::shared_ptr<Player> player)
{
    if (eTYPE_DERIVED_FROM(EntityType::MULE, GetType()) || isBaby() || getInLoveValue() >= 1)
        return 0;

    if (mLevel->canCreateMore(GetType(), Level::eSpawnType_Breed))
    {
        if (!mLevel->mIsClientSide)
            setInLove(player);
        return 1;
    }

    player->displayClientMessage(0xB399BD15, true);
    return 0;
}

int AbstractHorse::handleEating(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item)
{
    float healAmount = 0.0f;
    int flag = 0;
    int growth = 0;
    int temperGain = 0;
    Item* food = item->getItem();

    if (food == Items::wheat)
    {
        healAmount = 2.0f;
        growth = 20;
        temperGain = 3;
    }
    else if (food == Items::sugar)
    {
        healAmount = 1.0f;
        growth = 30;
        temperGain = 3;
    }
    else if (food == Item::byBlock(Blocks::hay_block))
    {
        healAmount = 20.0f;
        growth = 180;
    }
    else if (food == Items::apple)
    {
        healAmount = 3.0f;
        growth = 60;
        temperGain = 3;
    }
    else if (food == Items::goldenCarrot)
    {
        healAmount = 4.0f;
        growth = 60;
        temperGain = 5;
        if (isTamed() && !isBaby() && !isInLove() && !isBreedCoolingDown())
            flag = CheckAndSetInLove(player);
    }
    else if (food == Items::goldenApple)
    {
        healAmount = 10.0f;
        growth = 240;
        temperGain = 10;
        if (isTamed() && !isBaby() && !isInLove() && !isBreedCoolingDown())
            flag = CheckAndSetInLove(player);
    }

    if (getHealth() < getMaxHealth() && healAmount > 0.0f)
    {
        heal(healAmount);
        flag = 1;
    }

    if (isBaby() && growth != 0)
    {
        ageUp(growth, true);
        flag = 1;
    }

    if (temperGain != 0 && (flag != 0 || !isTamed()) && getTemper() < getMaxTemper())
    {
        flag = 1;
        modifyTemper(temperGain);
    }

    if (flag != 0)
        eating(Item::getId(food));

    return flag;
}

void AbstractHorse::doPlayerRide(const boost::shared_ptr<Player>& player)
{
    player->mYRot = mYRot;
    player->mXRot = mXRot;
    setEating(false);
    setStanding(false);
    if (mLevel->mIsClientSide)
        return;
    player->startRiding(shared_from_this());
}

bool AbstractHorse::isImmobile()
{
    return (LivingEntity::isImmobile() && isVehicle() && isSaddled()) || isEating() || isStanding();
}

bool AbstractHorse::isFood(const not_null_ptr<ItemInstance>&)
{
    return false;
}

void AbstractHorse::moveTail()
{
    mTailCounter = 1;
}

void AbstractHorse::die(const DamageSource* source)
{
    LivingEntity::die(source);
    if (mLevel->mIsClientSide || !mInventory)
        return;
    for (unsigned int slot = 0; slot < mInventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = mInventory->getItem(slot);
        if (!item->isEmpty())
            spawnAtLocation(item, 0.0f);
    }
}

void AbstractHorse::aiStep()
{
    if (mRandom->nextInt(200) == 0)
        moveTail();

    Animal::aiStep();

    if (mLevel->mIsClientSide)
        return;

    if (mRandom->nextInt(900) == 0 && mDeathTime == 0)
        heal(1.0f);

    if (mLevel->mIsClientSide)
    {
        if (mForcedAgeTimer > 0)
        {
            if ((mForcedAgeTimer & 3) == 0)
            {
                mLevel->addParticle(
                    PARTICLE_HAPPY_VILLAGER,
                    mX + static_cast<double>(mRandom->nextFloat() * mBbWidth * 2.0f) - static_cast<double>(mBbWidth),
                    mY + 0.5 + static_cast<double>(mRandom->nextFloat() * mBbHeight),
                    mZ + static_cast<double>(mRandom->nextFloat() * mBbWidth * 2.0f) - static_cast<double>(mBbWidth),
                    0.0, 0.0, 0.0, arrayWithLength<int>());
            }
            --mForcedAgeTimer;
        }
    }

    if (canEatGrass())
    {
        if (!isEating() && !isVehicle() && mRandom->nextInt(300) == 0 && !mGoalSelector.isUsingGoal(mPanicGoal))
        {
            Block* block =
                mLevel->getBlockState(BlockPos(std::floor(mX), std::floor(mY) - 1.0, std::floor(mZ)))->getBlock();
            if (block == Blocks::grass)
                setEating(true);
        }
        if (isEating() && ++mEatingCounter > 50)
        {
            mEatingCounter = 0;
            setEating(false);
        }
    }

    followMommy();
}

void AbstractHorse::followMommy()
{
    if (!isBred() || !isBaby() || isEating())
        return;
    boost::shared_ptr<AbstractHorse> mommy = getClosestMommy(shared_from_this(), 16.0);
    if (mommy && distanceToSqr(mommy) > 4.0)
        mNavigation->createPath(mommy);
}

bool AbstractHorse::canEatGrass() const
{
    return true;
}

void AbstractHorse::tick()
{
    PIXBeginNamedEvent(0.0f, "Abstract Horse tick");

    Mob::tick();

    if (mMouthCounter > 0 && ++mMouthCounter > 30)
    {
        mMouthCounter = 0;
        setFlag(64, false);
    }

    if (mStandCounter > 0 && (mLevel->mIsClientSide || ++mStandCounter > 20))
    {
        mStandCounter = 0;
        setStanding(false);
    }

    if (mTailCounter > 0 && ++mTailCounter > 8)
        mTailCounter = 0;

    if (mSprintCounter > 0 && ++mSprintCounter > 300)
        mSprintCounter = 0;

    mEatAnimO = mEatAnim;
    if (isEating())
    {
        mEatAnim += (1.0f - mEatAnim) * 0.4f + 0.05f;
        if (mEatAnim > 1.0f)
            mEatAnim = 1.0f;
    }
    else
    {
        mEatAnim += (0.0f - mEatAnim) * 0.4f - 0.05f;
        if (mEatAnim < 0.0f)
            mEatAnim = 0.0f;
    }

    mStandAnimO = mStandAnim;
    if (isStanding())
    {
        mEatAnim = 0.0f;
        mEatAnimO = 0.0f;
        mStandAnim += (1.0f - mStandAnim) * 0.4f + 0.05f;
        if (mStandAnim > 1.0f)
            mStandAnim = 1.0f;
    }
    else
    {
        mAllowStandSliding = false;
        mStandAnim += (0.8f * mStandAnim * mStandAnim * mStandAnim - mStandAnim) * 0.6f - 0.05f;
        if (mStandAnim < 0.0f)
            mStandAnim = 0.0f;
    }

    mMouthAnimO = mMouthAnim;
    if (getFlag(64))
    {
        mMouthAnim += (1.0f - mMouthAnim) * 0.7f + 0.05f;
        if (mMouthAnim > 1.0f)
            mMouthAnim = 1.0f;
    }
    else
    {
        mMouthAnim += (0.0f - mMouthAnim) * 0.7f - 0.05f;
        if (mMouthAnim < 0.0f)
            mMouthAnim = 0.0f;
    }

    PIXEndNamedEvent();
}

void AbstractHorse::makeMad()
{
    stand();
    const SoundEvent* sound = getAngrySound();
    const long long now = System::processTimeInMilliSecs();
    if (sound != NULL && mLastAngrySoundTime + 1000 < now)
    {
        mLastAngrySoundTime = now;
        playSound(sound, getSoundVolume(), getVoicePitch());
    }
}

bool AbstractHorse::tameWithName(const boost::shared_ptr<Player>& player)
{
    setOwnerUUID(player->getUUID());
    setTamed(true);
    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_SUCCEEDED, 0);
    if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, player->GetType()))
    {
        player->awardStat(GenericStats::tamedEntity(EntityType::HORSE),
                          GenericStats::param_tamedEntity(EntityType::HORSE));
    }
    return true;
}

void AbstractHorse::travel(float x, float y, float z)
{
    if (!isVehicle() || !canBeControlledByRider()
        || (!isSaddled() && !eTYPE_DERIVED_FROM(EntityType::SKELETON_HORSE, GetType())))
    {
        mFlyingSpeed = 0.02f;
        LivingEntity::travel(x, y, z);
        return;
    }

    PIXBeginNamedEvent(0.0f, "Horse::travel");

    boost::shared_ptr<LivingEntity> rider = boost::static_pointer_cast<LivingEntity>(getControllingPassenger());

    mYRotO = mYRot;
    mYRot = rider->mYRot;
    mXRot = rider->mXRot * 0.5f;
    setRot(mYRot, mXRot);
    mYBodyRot = mYRot;
    mYHeadRot = mYRot;

    float strafe = rider->mXxa * 0.5f;
    float forward = rider->mZza;
    if (forward <= 0.0f)
    {
        forward = forward * 0.25f;
        mGallopSoundCounter = 0;
    }

    if (mOnGround && mJumpRidingScale == 0.0f && isStanding() && !mAllowStandSliding && !isJumping())
    {
        strafe = 0.0f;
        forward = 0.0f;
    }

    if (mJumpRidingScale > 0.0f && !isJumping() && mOnGround)
    {
        mYd = getCustomJump() * static_cast<double>(mJumpRidingScale);
        if (hasEffect(MobEffects::JUMP))
            mYd += static_cast<double>(static_cast<float>(getEffect(MobEffects::JUMP)->getAmplifier() + 1) * 0.1f);
        setIsJumping(true);
        mHasImpulse = true;
        if (forward > 0.0f)
        {
            mXd += static_cast<double>(-0.4f * Mth::sin(mYRot * Mth::DEG_TO_RAD) * mJumpRidingScale);
            mZd += static_cast<double>(0.4f * Mth::cos(mYRot * Mth::DEG_TO_RAD) * mJumpRidingScale);
        }
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, rider->GetType()))
        {
            mLevel->playLocalSound(mX, mY, mZ, getJumpSound(), SoundSource::eSoundSource_Players, 0.4f, 1.0f, false,
                                   16.0f);
        }
        mJumpRidingScale = 0.0f;
    }

    mMaxUpStep = 1.0f;
    mFlyingSpeed = getSpeed() * 0.1f;

    if (isControlledByLocalInstance())
    {
        setSpeed(static_cast<float>(getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
        LivingEntity::travel(strafe, y, forward);
    }
    else if (eTYPE_DERIVED_FROM(EntityType::PLAYER, rider->GetType()))
    {
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
    }

    if (mOnGround)
    {
        mJumpRidingScale = 0.0f;
        if (isJumping())
            setStanding(false);
        setIsJumping(false);
    }

    updateModelAnimation();

    PIXEndNamedEvent();
}

void AbstractHorse::updateModelAnimation()
{
    const double dz = mZ - mZo;
    const double dx = mX - mXo;
    mAnimationSpeedOld = mAnimationSpeed;

    float step = Mth::sqrt(dx * dx + dz * dz) * 4.0f;
    if (step <= 1.0e-05f)
        step = Mth::subtractFromLarger(mYRot, mYRotO);

    if (step > 1.0f)
        step = 1.0f;
    else if (step > 0.0f && step < 0.32f)
        step = 0.32f;

    mAnimationSpeed += (step - mAnimationSpeed) * 0.4f;
    mAnimationPosition += mAnimationSpeed;
}

void AbstractHorse::addDataWalkers(DataFixerUpper* fixer, unsigned long long type)
{
    Mob::addDataWalkers(fixer, type);
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, new ItemDataWalker(type, L"SaddleItem"));
}

void AbstractHorse::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putBoolean(L"EatingHaystack", isEating());
    tag->putBoolean(L"Bred", isBred());
    tag->putInt(L"Temper", getTemper());
    tag->putBoolean(L"Tame", isTamed());
    if (getOwnerUUID().empty())
        tag->putString(L"OwnerUUID", getOwnerUUID());

    if (!mInventory->getItem(0)->isEmpty())
        tag->put(L"SaddleItem", mInventory->getItem(0)->save(new CompoundTag()));
}

void AbstractHorse::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    setEating(tag->getBoolean(L"EatingHaystack"));
    setBred(tag->getBoolean(L"Bred"));
    setTemper(tag->getInt(L"Temper"));
    setTamed(tag->getBoolean(L"Tame"));

    std::wstring owner(L"");
    if (tag->contains(L"OwnerUUID", Tag::TAG_STRING))
        owner = tag->getString(L"OwnerUUID");
    else
    {
        std::wstring ownerName = tag->getString(L"OwnerName");
    }
    if (owner.empty())
        setOwnerUUID(owner);

    if (tag->contains(L"SaddleItem", Tag::TAG_COMPOUND))
    {
        not_null_ptr<ItemInstance> saddle(new ItemInstance(tag->getCompound(L"SaddleItem")));
        if (saddle->getItem() == Items::saddle)
            mInventory->setItem(0, saddle);
    }
    else if (tag->getBoolean(L"Saddle"))
    {
        mInventory->setItem(0, not_null_ptr<ItemInstance>(new ItemInstance(Items::saddle)));
    }

    updateEquipment();
}

bool AbstractHorse::canMate(const boost::shared_ptr<Animal>&)
{
    return false;
}

bool AbstractHorse::canParent()
{
    return !isPassenger() && isTamed() && !isBaby() && getHealth() >= getMaxHealth() && isInLove();
}

boost::shared_ptr<AgableMob> AbstractHorse::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    (void)partner;
    return boost::shared_ptr<AgableMob>();
}

double AbstractHorse::generateRandomMaxHealth()
{
    return static_cast<double>(static_cast<float>(mRandom->nextInt(8)) + 15.0f
                               + static_cast<float>(mRandom->nextInt(9)));
}

double AbstractHorse::generateRandomJumpStrength()
{
    return mRandom->nextDouble() * 0.2 + 0.4000000059604645 + mRandom->nextDouble() * 0.2 + mRandom->nextDouble() * 0.2;
}

double AbstractHorse::generateRandomSpeed()
{
    return (mRandom->nextDouble() * 0.3 + 0.44999998807907104 + mRandom->nextDouble() * 0.3
            + mRandom->nextDouble() * 0.3)
           * 0.25;
}

void AbstractHorse::setOffspringAttributes(const boost::shared_ptr<AgableMob>& other,
                                           const boost::shared_ptr<AbstractHorse>& offspring)
{
    const double health = getAttribute(SharedMonsterAttributes::MAX_HEALTH)->getBaseValue()
                          + other->getAttribute(SharedMonsterAttributes::MAX_HEALTH)->getBaseValue()
                          + generateRandomMaxHealth();
    offspring->getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(health / 3.0);

    const double jump = getAttribute(JUMP_STRENGTH)->getBaseValue() + other->getAttribute(JUMP_STRENGTH)->getBaseValue()
                        + generateRandomJumpStrength();
    offspring->getAttribute(JUMP_STRENGTH)->setBaseValue(jump / 3.0);

    const double speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getBaseValue()
                         + other->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getBaseValue()
                         + generateRandomSpeed();
    offspring->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(speed / 3.0);
}

bool AbstractHorse::canBeControlledByRider()
{
    return ((isTamed() && isSaddled()) || eTYPE_DERIVED_FROM(EntityType::SKELETON_HORSE, GetType()))
           && Entity::instanceof(getControllingPassenger(), EntityType::LIVING_ENTITY);
}

float AbstractHorse::getEatAnim(float partialTick) const
{
    return (mEatAnim - mEatAnimO) * partialTick + mEatAnimO;
}

float AbstractHorse::getStandAnim(float partialTick) const
{
    return (mStandAnim - mStandAnimO) * partialTick + mStandAnimO;
}

float AbstractHorse::getMouthAnim(float partialTick) const
{
    return (mMouthAnim - mMouthAnimO) * partialTick + mMouthAnimO;
}

void AbstractHorse::spawnTamingParticles(bool success)
{
    const ParticleType* particle = success ? PARTICLE_HEART : PARTICLE_SMOKE;
    for (int i = 0; i < 7; ++i)
    {
        const double xa = mRandom->nextGaussian() * 0.02;
        const double ya = mRandom->nextGaussian() * 0.02;
        const double za = mRandom->nextGaussian() * 0.02;
        mLevel->addParticle(
            particle, mX + static_cast<double>(mRandom->nextFloat() * mBbWidth * 2.0f) - static_cast<double>(mBbWidth),
            mY + 0.5 + static_cast<double>(mRandom->nextFloat() * mBbHeight),
            mZ + static_cast<double>(mRandom->nextFloat() * mBbWidth * 2.0f) - static_cast<double>(mBbWidth), xa, ya,
            za, arrayWithLength<int>());
    }
}

void AbstractHorse::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::TAMING_FAILED)
        spawnTamingParticles(false);
    else if (event == EntityEvent::TAMING_SUCCEEDED)
        spawnTamingParticles(true);
    else
        Animal::handleEntityEvent(event, data);
}

void AbstractHorse::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!RiderPositionOverrides::OverridePosition(shared_from_this(), passenger))
        Entity::positionRider(passenger);

    if (eTYPE_DERIVED_FROM(EntityType::MOB, passenger->GetType()))
        mYBodyRot = boost::dynamic_pointer_cast<Mob>(passenger)->mYBodyRot;

    if (mStandAnimO <= 0.0f)
        return;

    const float sin = Mth::sin(mYBodyRot * 3.1415927f / 180.0f);
    const float cos = Mth::cos(mYBodyRot * 3.1415927f / 180.0f);
    const float offset = 0.7f * mStandAnimO;
    const float rise = 0.15f * mStandAnimO;

    passenger->setPos(mX + static_cast<double>(offset * sin),
                      mY + getRideHeight() + passenger->getRidingHeight() + static_cast<double>(rise),
                      mZ - static_cast<double>(offset * cos));

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, passenger->GetType()))
        boost::dynamic_pointer_cast<LivingEntity>(passenger)->mYBodyRot = mYBodyRot;
}

bool AbstractHorse::onLadder()
{
    return false;
}
float AbstractHorse::getEyeHeight()
{
    return mBbHeight;
}
bool AbstractHorse::wearsArmor()
{
    return false;
}
bool AbstractHorse::isArmor(const not_null_ptr<ItemInstance>&)
{
    return false;
}

bool AbstractHorse::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    const unsigned int armourSlot = static_cast<unsigned int>(slot - 400);
    if (armourSlot < 2 && static_cast<int>(armourSlot) < static_cast<int>(mInventory->getContainerSize()))
    {
        if (armourSlot == 0 && item->getItem() != Items::saddle)
            return false;
        if (armourSlot == 1 && (!wearsArmor() || !isArmor(item)))
            return false;
        mInventory->setItem(armourSlot, item);
        updateEquipment();
        return true;
    }

    const int chestSlot = slot - 500 + 2;
    if (chestSlot > 1 && chestSlot < static_cast<int>(mInventory->getContainerSize()))
    {
        mInventory->setItem(static_cast<unsigned int>(chestSlot), item);
        return true;
    }

    return false;
}

boost::shared_ptr<Entity> AbstractHorse::getControllingPassenger()
{
    if (getPassengers()->empty())
        return boost::shared_ptr<Entity>();
    const std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    return passengers->at(0).lock();
}

MobGroupData* AbstractHorse::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                              const std::wstring&)
{
    MobGroupData* spawned = Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
    if (mRandom->nextInt(5) == 0)
        setAge(-24000);
    return spawned;
}

bool AbstractHorse::removeWhenFarAway()
{
    return Animal::removeWhenFarAway() && !isTamed() && !isSaddled() && !isPassenger();
}

unsigned long long AbstractHorse::GetType()
{
    return EntityType::ABSTRACT_HORSE;
}
