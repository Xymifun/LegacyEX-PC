#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/AttributeInstance.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/ai/FleeSunGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RangedBowAttackGoal.hpp"
#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/core/Turtle.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/world/block/Blocks.hpp"

const EntityDataAccessor<bool>* AbstractSkeleton::DATA_ID_ATTACKING = NULL;

AbstractSkeleton::AbstractSkeleton(Level* level)
    : Monster(level),
      mMeleeGoal(new SkeleteMeleeAttackGoal(this)),
      mBowGoal(new RangedBowAttackGoal(this, 1.0, 20, 15.0f))
{
    setSize(0.6f, 1.99f);
    mReassessWeaponGoal = true;
    mInWater = false;
    mCategories |= EntityCategory::eEntityCategory_Undead | EntityCategory::eEntityCategory_Humanoid
                   | EntityCategory::eEntityCategory_Monster;
}

AbstractSkeleton::~AbstractSkeleton()
{
    delete mBowGoal;
    delete mMeleeGoal;
}

void AbstractSkeleton::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    boost::shared_ptr<Arrow> arrow = getArrow(distanceFactor);
    const double dx = target->mX - mX;
    const double targetY = target->mBoundingBox->mMinY + target->mBbHeight / 3.0f - arrow->mY;
    const double dz = target->mZ - mZ;
    const double horizontalDistance = std::sqrt(dx * dx + dz * dz);

    arrow->shoot(dx, targetY + horizontalDistance * 0.2f, dz, 1.6f,
                 14.0f - static_cast<float>(mLevel->getDifficulty()->getId()) * 4.0f);
    playSound(SoundEvents::SKELETON_SHOOT, 1.0f, 1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
    mLevel->addEntity(arrow);
}

bool AbstractSkeleton::isAggressive()
{
    return mEntityData->get(DATA_ID_ATTACKING);
}

void AbstractSkeleton::setAggressive(bool aggressive)
{
    mEntityData->set(DATA_ID_ATTACKING, aggressive, true);
}

void AbstractSkeleton::staticCtor()
{
    DATA_ID_ATTACKING = SynchedEntityData::defineId<bool>(EntityType::ABSTRACT_SKELETON, EntityType::MONSTER,
                                                          EntityDataSerializers::BOOLEAN);
}

void AbstractSkeleton::reassessWeaponGoal()
{
    mReassessWeaponGoal = false;
    if (mLevel == NULL || mLevel->mIsClientSide)
    {
        return;
    }

    BlockPos pos(mX, mY, mZ);
    if (!mLevel->hasChunkAt(pos) || !mLevel->getChunkAt(pos)->isTerrainPopulated())
    {
        mReassessWeaponGoal = true;
        return;
    }

    mGoalSelector.removeGoal(mMeleeGoal);
    mGoalSelector.removeGoal(mBowGoal);

    not_null_ptr<ItemInstance> item = getMainHandItem();
    if (item->getItem() == Items::bow && !isUnderLiquid(Material::WATER))
    {
        int interval = 20;
        if (mLevel->getDifficulty() != Difficulty::HARD)
        {
            interval = 40;
        }
        mBowGoal->setMinAttackInterval(interval);
        mGoalSelector.addGoal(4, mBowGoal, false);
    }
    else
    {
        mGoalSelector.addGoal(4, mMeleeGoal, false);
    }
}

void AbstractSkeleton::tick()
{
    Monster::tick();
    if (mReassessWeaponGoal)
        reassessWeaponGoal();
}

void AbstractSkeleton::registerGoals()
{
    mGoalSelector.addGoal(5, new SinkInWaterRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, new FleeSunGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(6, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
    mTargetSelector.addGoal(3,
                            new NearestAttackableTargetGoal(this, typeid(Turtle), 10, true, false,
                                                            Turtle::BABY_ON_LAND_SELECTOR, false, false),
                            true);
}

void AbstractSkeleton::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
}

void AbstractSkeleton::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_ID_ATTACKING, false);
}

void AbstractSkeleton::playStepSound(const BlockPos& pos, Block* block)
{
    playSound(getStepSound(), 0.15f, 1.0f);
}

EMobType AbstractSkeleton::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

void AbstractSkeleton::aiStep()
{
    if (!mLevel->mIsClientSide && mLevel->isDay())
    {
        float brightness = getBrightness();
        BlockPos pos = Entity::instanceof(getVehicle(), EntityType::BOAT)
                           ? BlockPos(mX, (double)Math::round(mY), mZ).above()
                           : BlockPos(mX, (double)Math::round(mY), mZ);

        if (brightness > 0.5f && mRandom->nextFloat() * 30.0f < (brightness - 0.4f) * 2.0f && mLevel->canSeeSky(pos))
        {
            not_null_ptr<ItemInstance> item = getItemSlot(EquipmentSlot::HEAD);
            if (!item->isEmpty())
            {
                if (item->isDamageableItem())
                {
                    item->setAuxValue(item->getDamageValue() + mRandom->nextInt(2));
                    if (item->getDamageValue() >= item->getMaxDamage())
                    {
                        breakItem(item);
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
    bool inWater = isUnderLiquid(Material::WATER);
    if (mInWater)
    {
        if (!inWater)
            mReassessWeaponGoal = true;
    }
    else if (inWater)
        mReassessWeaponGoal = true;
    mInWater = inWater;
    Monster::aiStep();
}

void AbstractSkeleton::rideTick()
{
    LivingEntity::rideTick();
    boost::shared_ptr<Entity> vehicle = getVehicle();
    if (vehicle != NULL && vehicle->instanceof(EntityType::PATHFINDER_MOB))
    {
        boost::shared_ptr<PathfinderMob> pathfinder = boost::static_pointer_cast<PathfinderMob>(getVehicle());
        mYBodyRot = pathfinder->mYBodyRot;
    }
}

void AbstractSkeleton::die(const DamageSource* source)
{
    LivingEntity::die(source);

    if (source->getDirectEntity() && source->getEntity())
    {
        if (source->getDirectEntity()->instanceof(EntityType::ARROW)
            && source->getEntity()->instanceof(EntityType::PLAYER))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(source->getEntity());
            const double dx = player->mX - mX;
            const double dz = player->mZ - mZ;
            if (dx * dx + dz * dz >= 2500.0)
                player->awardStat(GenericStats::snipeSkeleton(), GenericStats::param_snipeSkeleton());
        }
    }
}

void AbstractSkeleton::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
}

MobGroupData* AbstractSkeleton::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                                 const std::wstring& dataString)
{
    data = Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
    populateDefaultEquipmentSlots(difficulty);
    populateDefaultEquipmentEnchantments(difficulty);
    mReassessWeaponGoal = true;
    setCanPickUpLoot(mRandom->nextFloat() < 0.55f * difficulty.getSpecialMultiplier());

    bool hasNoHelmet = getItemSlot(EquipmentSlot::HEAD) == ItemInstance::EMPTY;
    if (hasNoHelmet && SharedConstants::IS_HALLOWEEN && mRandom->nextFloat() < 0.25f)
    {
        setItemSlot(EquipmentSlot::HEAD,
                    not_null_ptr<ItemInstance>(
                        new ItemInstance(mRandom->nextFloat() < 0.1f ? Blocks::lit_pumpkin : Blocks::carved_pumpkin)));
        mArmorDropChances[EquipmentSlot::HEAD->getIndex()] = 0.0f;
    }
    return data;
}

boost::shared_ptr<Arrow> AbstractSkeleton::getArrow(float distanceFactor)
{
    boost::shared_ptr<TippableArrow> arrow(
        new TippableArrow(mLevel, boost::static_pointer_cast<LivingEntity>(shared_from_this())));
    arrow->setEnchantmentEffectsFromEntity(boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                           distanceFactor);
    return arrow;
}

void AbstractSkeleton::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    mReassessWeaponGoal = true;
}

void AbstractSkeleton::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> itemInstance)
{
    Mob::setItemSlot(slot, itemInstance);
    if (!mLevel->mIsClientSide && slot == EquipmentSlot::MAINHAND)
        mReassessWeaponGoal = true;
}

float AbstractSkeleton::getEyeHeight()
{
    float height = 1.62f;
    return height;
}

double AbstractSkeleton::getRidingHeight()
{
    return -0.3;
}

bool AbstractSkeleton::breathesWater()
{
    return true;
}

unsigned long long AbstractSkeleton::GetType()
{
    return EntityType::ABSTRACT_SKELETON;
}

AbstractSkeleton::SkeleteMeleeAttackGoal::SkeleteMeleeAttackGoal(AbstractSkeleton* skeleton)
    : MeleeAttackGoal(skeleton, 1.2, false)
{
}

void AbstractSkeleton::SkeleteMeleeAttackGoal::start()
{
    MeleeAttackGoal::start();
    static_cast<AbstractSkeleton*>(mMob)->setAggressive(true);
}

void AbstractSkeleton::SkeleteMeleeAttackGoal::stop()
{
    MeleeAttackGoal::stop();
    static_cast<AbstractSkeleton*>(mMob)->setAggressive(false);
}
