#include "game/entity/mob/Zombie.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Mth.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/FindMountGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/BreakDoorGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/core/Drowned.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MoveThroughVillageGoal.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/core/Turtle.hpp"
#include "game/world/generation/ZombieVillager.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/RemoveBlockGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/RangedAttribute.hpp"
#include "game/core/BaseAttributeMap.hpp"

static const EntityDataAccessor<bool>* gZombieBabyId = NULL;
static const EntityDataAccessor<bool>* gZombieAggressiveId = NULL;
static const EntityDataAccessor<bool>* gZombieConvertingId = NULL;

Attribute* Zombie::SPAWN_REINFORCEMENTS_CHANCE = new RangedAttribute(NULL, eATTRIBUTE_ID_ZombieSpawnReinforcements, 0.0,
                                                                     0.0, 1.0);

AttributeModifier* Zombie::SPEED_MODIFIER_BABY = new AttributeModifier(eMODIFIER_ID_BabyZombieSpeedBoost, 0.5, 1);

Zombie::ZombieGroupData::ZombieGroupData(bool isBaby, bool isJockey, bool isVillager)
    : mIsBaby(isBaby),
      mIsJockey(isJockey),
      mIsVillager(isVillager)
{
}

Zombie::ZombieGroupData::~ZombieGroupData()
{
}

Zombie::Zombie(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    _init();
}

Zombie::Zombie(Level* level, bool initializeGoals) : Monster(level)
{
    _init();
    if (initializeGoals)
        fjDerivedCtorCalls();
}

Zombie::~Zombie()
{
    delete mBreakDoorGoal;
}

Entity* Zombie::create(Level* level)
{
    return new Zombie(level);
}

void Zombie::staticCtor()
{
    gZombieBabyId = SynchedEntityData::defineId<bool>(EntityType::ZOMBIE, EntityType::MONSTER,
                                                      EntityDataSerializers::BOOLEAN);
    gZombieAggressiveId = SynchedEntityData::defineId<bool>(EntityType::ZOMBIE, EntityType::MONSTER,
                                                            EntityDataSerializers::BOOLEAN);
    gZombieConvertingId = SynchedEntityData::defineId<bool>(EntityType::ZOMBIE, EntityType::MONSTER,
                                                            EntityDataSerializers::BOOLEAN);
}

void Zombie::_init()
{
    mCanBreakDoors = false;
    mBaseWidth = -1.0f;
    mBaseHeight = 0.0f;
    mBreakDoorGoal = new BreakDoorGoal(this);
    setSize(0.6f, 1.95f);
    mIsJockey = false;
    mConversionTime = 0;
    mWaterConversionTime = 0;
    mCategories |= EntityCategory::eEntityCategory_Zombie | EntityCategory::eEntityCategory_Humanoid
                   | EntityCategory::eEntityCategory_Monster;
}

void Zombie::registerGoals()
{
    mGoalSelector.addGoal(2, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(4,
                          new RemoveBlockGoal(Blocks::turtle_egg, this, 1.0, 16, 2, 1.6, SoundEvents::TURTLE_EGG_HATCH,
                                              SoundEvents::TURTLE_EGG_BREAK),
                          true);
    mGoalSelector.addGoal(5, new MoveTowardsRestrictionGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new SinkInWaterRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    addBehaviourGoals();
}

void Zombie::addBehaviourGoals()
{
    mGoalSelector.addGoal(6, new MoveThroughVillageGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(1, new FindMountGoal(*this, 16.0f, 0, false, false, -1.0f), true);
    arrayWithLength<unsigned long long> entityTypes(1, true);
    entityTypes.data[0] = EntityType::PIG_ZOMBIE;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, entityTypes), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Villager), false, false, false), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), true, false, false), true);
    mTargetSelector.addGoal(5,
                            new NearestAttackableTargetGoal(this, typeid(Turtle), 10, true, false,
                                                            Turtle::BABY_ON_LAND_SELECTOR, false, false),
                            true);
}

void Zombie::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23f);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(3.0);
    getAttribute(SharedMonsterAttributes::ARMOR)->setBaseValue(2.0);
    getAttributes()->registerAttribute(SPAWN_REINFORCEMENTS_CHANCE)->setBaseValue(mRandom->nextDouble() * 0.1f);
}

void Zombie::defineSynchedData()
{
    Monster::defineSynchedData();
    getEntityData()->define(gZombieBabyId, false);
    getEntityData()->define(gZombieAggressiveId, false);
    getEntityData()->define(gZombieConvertingId, false);
}

void Zombie::setAggressive(bool aggressive)
{
    getEntityData()->set(gZombieAggressiveId, aggressive, true);
}

bool Zombie::isAggressive()
{
    return getEntityData()->get(gZombieAggressiveId);
}

bool Zombie::canBreakDoors()
{
    return mCanBreakDoors;
}

void Zombie::setCanBreakDoors(bool canBreakDoors)
{
    if (mCanBreakDoors != canBreakDoors)
    {
        mCanBreakDoors = canBreakDoors;
        static_cast<GroundPathNavigation*>(getNavigation())->setCanOpenDoors(canBreakDoors);
        if (canBreakDoors)
            mGoalSelector.addGoal(1, mBreakDoorGoal, false);
        else
            mGoalSelector.removeGoal(mBreakDoorGoal);
    }
}

bool Zombie::isBaby()
{
    return getEntityData()->get(gZombieBabyId);
}

bool Zombie::isJockey()
{
    return mIsJockey;
}

int Zombie::getExperienceReward(boost::shared_ptr<Player> player)
{
    if (isBaby())
        mExperienceReward = (int)((float)mExperienceReward * 2.5f);
    return Mob::getExperienceReward(player);
}

void Zombie::setBaby(bool baby)
{
    getEntityData()->set(gZombieBabyId, baby, true);
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
        speed->removeModifier(SPEED_MODIFIER_BABY);
        if (baby)
            speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_BABY));
    }
    internalSetSize(baby ? 0.5f : 1.0f);
}

void Zombie::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (accessor->getId() == gZombieBabyId->getId())
    {
        float scale = isBaby() ? 0.5f : 1.0f;
        internalSetSize(scale);
    }
    LivingEntity::onSyncedDataUpdated(accessor);
}

void Zombie::aiStep()
{
    if (mLevel->isDay() && !mLevel->mIsClientSide && isSunSensitive())
    {
        float brightness = getBrightness();
        if (brightness > 0.5f && mRandom->nextFloat() * 30.0f < (brightness - 0.4f) * 2.0f)
        {
            if (mLevel->canSeeSky(BlockPos(mX, mY + getEyeHeight(), mZ)))
            {
                not_null_ptr<ItemInstance> helmet = getItemSlot(EquipmentSlot::HEAD);
                if (!helmet->isEmpty())
                {
                    if (helmet->isDamageableItem())
                    {
                        helmet->setAuxValue(helmet->getDamageValue() + mRandom->nextInt(2));
                        if (helmet->getDamageValue() >= helmet->getMaxDamage())
                        {
                            breakItem(helmet);
                            setItemSlot(EquipmentSlot::HEAD, ItemInstance::EMPTY);
                        }
                    }
                }
                else
                {
                    setSecondsOnFire(8);
                }
            }
        }
    }
    Monster::aiStep();
    if (isBaby())
        internalSetSize(0.5f);
    else
        internalSetSize(1.0f);
}

bool Zombie::hurt(const DamageSource* source, float damage)
{
    if (!Monster::hurt(source, damage))
        return false;

    boost::shared_ptr<LivingEntity> attacker = getTarget();
    if (!attacker && Entity::instanceof(source->getEntity(), EntityType::LIVING_ENTITY))
        attacker = boost::static_pointer_cast<LivingEntity>(source->getEntity());

    if (attacker && mLevel->getDifficulty() == Difficulty::HARD
        && mRandom->nextFloat() < getAttribute(SPAWN_REINFORCEMENTS_CHANCE)->getValue()
        && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBSPAWNING))
    {
        int blockX = Mth::floor(mX);
        int blockY = Mth::floor(mY);
        int blockZ = Mth::floor(mZ);

        boost::shared_ptr<Zombie> reinforcement(new Zombie(mLevel));
        for (int attempt = 0; attempt < 50; ++attempt)
        {
            int spawnX = blockX + Mth::nextInt(mRandom, 7, 40) * Mth::nextInt(mRandom, -1, 1);
            int spawnY = blockY + Mth::nextInt(mRandom, 7, 40) * Mth::nextInt(mRandom, -1, 1);
            int spawnZ = blockZ + Mth::nextInt(mRandom, 7, 40) * Mth::nextInt(mRandom, -1, 1);
            if (mLevel->getBlockState(BlockPos(spawnX, spawnY - 1, spawnZ))->isTopSolidBlocking()
                && mLevel->getRawBrightness(BlockPos(spawnX, spawnY, spawnZ)) < 10)
            {
                reinforcement->setPos((double)spawnX, (double)spawnY, (double)spawnZ);
                if (!mLevel->hasNearPlayer((double)spawnX, (double)spawnY, (double)spawnZ, 7.0)
                    && mLevel->isUnobstructed(reinforcement->mBoundingBox, reinforcement)
                    && mLevel->getCollisionAABBs(reinforcement, reinforcement->mBoundingBox, false, false, false)
                           ->empty()
                    && !mLevel->containsAnyLiquid(reinforcement->mBoundingBox))
                {
                    mLevel->addEntity(reinforcement);
                    reinforcement->setTarget(attacker);
                    reinforcement->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(reinforcement)), NULL,
                                                    std::wstring(L""));
                    getAttribute(SPAWN_REINFORCEMENTS_CHANCE)->addModifier(new AttributeModifier(-0.05f, 0));
                    reinforcement->getAttribute(SPAWN_REINFORCEMENTS_CHANCE)
                        ->addModifier(new AttributeModifier(-0.05f, 0));
                    break;
                }
            }
        }
    }

    return true;
}

bool Zombie::doHurtTarget(boost::shared_ptr<Entity> target)
{
    bool result = Monster::doHurtTarget(target);
    if (result)
    {
        float difficulty = mLevel->getCurrentDifficultyAt(BlockPos(this)).getEffectiveDifficulty();
        bool emptyHand = getMainHandItem()->isEmpty();
        if (emptyHand)
        {
            if (isOnFire() && mRandom->nextFloat() < difficulty * 0.3f)
            {
                target->setSecondsOnFire((int)difficulty << 1);
            }
        }
    }
    return result;
}

bool Zombie::isSunSensitive() const
{
    return true;
}

const SoundEvent* Zombie::getAmbientSound()
{
    return SoundEvents::ZOMBIE_AMBIENT;
}

const SoundEvent* Zombie::getHurtSound(const DamageSource*)
{
    return SoundEvents::ZOMBIE_HURT;
}

const SoundEvent* Zombie::getDeathSound()
{
    return SoundEvents::ZOMBIE_DEATH;
}

const SoundEvent* Zombie::getStepSound()
{
    return SoundEvents::ZOMBIE_STEP;
}

void Zombie::playStepSound(const BlockPos&, Block*)
{
    playSound(getStepSound(), 0.15f, 1.0f);
}

EMobType Zombie::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

ResourceLocation* Zombie::getDefaultLootTable()
{
    return &BuiltInLootTables::ZOMBIE;
}

ResourceLocation* Zombie::getDefaultGearTable()
{
    return &BuiltInLootTables::ZOMBIE_EQUIPMENT;
}

void Zombie::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
}

void Zombie::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::ZOMBIE);
}

void Zombie::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (isBaby())
        tag->putBoolean(std::wstring(L"IsBaby"), true);
    tag->putBoolean(std::wstring(L"CanBreakDoors"), canBreakDoors());
    tag->putBoolean(std::wstring(L"IsJockey"), mIsJockey);
    tag->putInt(std::wstring(L"InWaterTime"), mWaterConversionTime);
    tag->putInt(std::wstring(L"DrownedConversionTime"), mConversionTime);
}

void Zombie::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->getBoolean(std::wstring(L"IsBaby")))
        setBaby(true);
    setCanBreakDoors(tag->getBoolean(std::wstring(L"CanBreakDoors")));
    mIsJockey = tag->getBoolean(std::wstring(L"IsJockey"));
    mConversionTime = tag->getInt(std::wstring(L"DrownedConversionTime"));
    getEntityData()->set(gZombieConvertingId, mConversionTime > 0, true);
    mWaterConversionTime = tag->getInt(std::wstring(L"InWaterTime"));
}

void Zombie::killed(boost::shared_ptr<LivingEntity> entity)
{
    Entity::killed(entity);

    if (mLevel->getDifficulty() == Difficulty::NORMAL || mLevel->getDifficulty() == Difficulty::HARD)
    {
        if (entity->GetType() == EntityType::VILLAGER
            && mLevel->canCreateMore(EntityType::ZOMBIE, Level::eSpawnType_Egg)
            && (mLevel->getDifficulty() == Difficulty::HARD || !mRandom->nextBoolean()))
        {
            boost::shared_ptr<Villager> villager = boost::static_pointer_cast<Villager>(entity);
            boost::shared_ptr<ZombieVillager> zombieVillager(new ZombieVillager(mLevel));

            zombieVillager->copyPosition(villager);
            mLevel->removeEntity(villager);

            zombieVillager->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(zombieVillager)),
                                             new ZombieGroupData(false, false, true), std::wstring(L""));

            zombieVillager->setProfession(villager->getProfession());
            zombieVillager->setBaby(villager->isBaby());
            zombieVillager->setNoAi(villager->isNoAi());
            if (villager->hasCustomName())
            {
                zombieVillager->setCustomName(villager->getCustomName(), villager->getOwnerUUID(),
                                              wasNamedByRestrictedPlayer());
                zombieVillager->setCustomNameVisible(villager->isCustomNameVisible());
            }

            mLevel->addEntity(zombieVillager);
            mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_ZOMBIE_INFECTED, BlockPos(this), 0);
        }
    }
}

float Zombie::getEyeHeight()
{
    float height = 1.62f;
    if (isBaby())
    {
        height -= 0.81f;
    }
    return height;
}

void Zombie::positionRider(const boost::shared_ptr<Entity>& entity)
{
    if (!hasPassenger(entity))
        return;

    if (RiderPositionOverrides::OverridePosition(shared_from_this(), entity))
        return;

    double sin = Mth::sin(mYBodyRot * Mth::DEG_TO_RAD);
    double cos = Mth::cos(mYBodyRot * Mth::DEG_TO_RAD);
    entity->setPos(mX + 0.35 * sin, mY + 1.3, mZ - 0.35 * cos);
}

bool Zombie::breathesWater()
{
    return true;
}

bool Zombie::canHoldItem(not_null_ptr<ItemInstance> item)
{
    if (item->getItem() == Items::egg && isBaby() && isPassenger())
        return false;
    return Mob::canHoldItem(item);
}

MobGroupData* Zombie::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& dataString)
{
    data = Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));

    const float specialMultiplier = difficulty.getSpecialMultiplier();
    setCanPickUpLoot(mRandom->nextFloat() < 0.55f * specialMultiplier);

    if (data == NULL)
    {
        Random* random = mLevel->mRandom;
        const bool isBaby = random->rollPercentageChance(5);
        const bool isJockey = isBaby && random->rollPercentageChance(15);
        const bool isAdult = random->rollPercentageChance(5);
        data = new ZombieGroupData(isBaby, isJockey, isAdult);
    }

    if (dynamic_cast<ZombieGroupData*>(data) != NULL)
    {
        ZombieGroupData* groupData = static_cast<ZombieGroupData*>(data);
        if (groupData->mIsBaby)
            setBaby(true);
        if (groupData->mIsJockey)
            mIsJockey = groupData->mIsJockey;
    }

    setCanBreakDoors(mRandom->nextFloat() < specialMultiplier * 0.1f);
    populateDefaultEquipmentSlots(difficulty);
    populateDefaultEquipmentEnchantments(difficulty);

    bool emptyHead = getItemSlot(EquipmentSlot::HEAD)->isEmpty();
    if (emptyHead && SharedConstants::IS_HALLOWEEN && mRandom->nextFloat() < 0.25f)
    {
        setItemSlot(EquipmentSlot::HEAD,
                    not_null_ptr<ItemInstance>(
                        new ItemInstance(mRandom->nextFloat() < 0.1f ? Blocks::lit_pumpkin : Blocks::carved_pumpkin)));
        mArmorDropChances[EquipmentSlot::HEAD->getIndex()] = 0.0f;
    }

    getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)
        ->addModifier(new AttributeModifier(mRandom->nextDouble() * 0.05f, 0));

    if (mRandom->nextFloat() < specialMultiplier * 0.05f)
    {
        getAttribute(SPAWN_REINFORCEMENTS_CHANCE)
            ->addModifier(new AttributeModifier(mRandom->nextDouble() * 0.25 + 0.5, 0));
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)
            ->addModifier(new AttributeModifier(mRandom->nextDouble() * 3.0 + 1.0, 2));
        setCanBreakDoors(true);
    }

    return data;
}

bool Zombie::isConverting()
{
    return getEntityData()->get(gZombieConvertingId);
}

void Zombie::startConverting(int conversionTime)
{
    mConversionTime = conversionTime;
    getEntityData()->set(gZombieConvertingId, true, true);
    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::ZOMBIE_CONVERTING, 0);
}

void Zombie::waterCheck()
{
    if (mLevel->mIsClientSide)
        return;
    if (GetType() != EntityType::ZOMBIE && GetType() != EntityType::HUSK)
        return;
    if (isUnderLiquid(Material::WATER))
    {
        ++mWaterConversionTime;
        if (!isConverting() && mWaterConversionTime >= 600)
            startConverting(300);
    }
    else
    {
        mWaterConversionTime = 0;
    }
}

void Zombie::tick()
{
    waterCheck();
    if (!mLevel->mIsClientSide && isConverting())
    {
        --mConversionTime;
        if (mConversionTime < 1)
        {
            finishConversion();
        }
    }
    Monster::tick();
}

void Zombie::setSize(float width, float height)
{
    bool initialized = mBaseWidth > 0.0f && mBaseHeight > 0.0f;
    mBaseWidth = width;
    mBaseHeight = height;
    if (!initialized)
        internalSetSize(1.0f);
}

void Zombie::internalSetSize(float scale)
{
    Entity::setSize(mBaseWidth * scale, mBaseHeight * scale);
}

not_null_ptr<ItemInstance> Zombie::getSkull()
{
    ItemInstance* skull = new ItemInstance(Items::skull, 1, 2);
    return not_null_ptr<ItemInstance>(skull);
}

double Zombie::getRidingHeight()
{
    return isBaby() ? 0.0 : -0.45;
}

void Zombie::die(const DamageSource* source)
{
    LivingEntity::die(source);
    if (source->getEntity() != NULL && eTYPE_DERIVED_FROM(EntityType::CREEPER, source->getEntity()->GetType()))
    {
        boost::shared_ptr<Creeper> creeper = boost::static_pointer_cast<Creeper>(source->getEntity());
        if (creeper->isPowered() && creeper->canDropMobsSkull())
        {
            creeper->increaseDroppedSkulls();
            if (!getSkull()->isEmpty())
                spawnAtLocation(getSkull(), 0.0f);
        }
    }
}

bool Zombie::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    bool result = Mob::startRiding(vehicle, force);
    if (isBaby() && force && eTYPE_DERIVED_FROM(EntityType::CHICKEN, vehicle->GetType()))
        mIsJockey = true;
    return result;
}

void Zombie::finishConversion()
{
    boost::shared_ptr<Drowned> drowned(new Drowned(mLevel));
    drowned->copyPosition(shared_from_this());

    if (isBaby())
    {
        drowned->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(drowned)),
                                  new ZombieGroupData(true, mIsJockey, false), std::wstring(L""));
    }
    else
    {
        drowned->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(drowned)),
                                  new ZombieGroupData(false, false, false), std::wstring(L""));
    }

    playSound(SoundEvents::DROWNED_CONVERTED, 1.0f, 1.0f);
    mLevel->removeEntity(shared_from_this());
    dropEquipment(true, 0);
    drowned->setNoAi(isNoAi());
    drowned->setCanPickUpLoot(canPickUpLoot());
    drowned->setCanBreakDoors(canBreakDoors());
    mLevel->addEntity(drowned);
}

unsigned long long Zombie::GetType()
{
    return EntityType::ZOMBIE;
}
