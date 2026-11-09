#include "game/core/Turtle.hpp"
#include <float.h>
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/LightningBolt.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Path.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/TurtleMoveControl.hpp"
#include "game/entity/ai/BabyOnLandSelector.hpp"
#include "game/entity/ai/TurtlePathNavigation.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/entity/ai/TurtleBreedGoal.hpp"
#include "game/entity/ai/TurtleGoHomeGoal.hpp"
#include "game/entity/ai/TurtleGoToWaterGoal.hpp"
#include "game/entity/ai/TurtleLayEggGoal.hpp"
#include "game/entity/ai/TurtlePanicGoal.hpp"
#include "game/entity/ai/TurtleRandomStrollGoal.hpp"
#include "game/entity/ai/TurtleTravelGoal.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<BlockPos>* Turtle::DATA_HOME_POS = NULL;
const EntityDataAccessor<bool>* Turtle::DATA_HAS_EGG = NULL;
const EntityDataAccessor<bool>* Turtle::DATA_HAS_GIVEN_UP_ON_EGG = NULL;
const EntityDataAccessor<bool>* Turtle::DATA_IS_LAYING_EGG = NULL;
const EntityDataAccessor<BlockPos>* Turtle::DATA_TRAVEL_POS = NULL;
const EntityDataAccessor<bool>* Turtle::DATA_GOING_HOME = NULL;
const EntityDataAccessor<bool>* Turtle::DATA_TRAVELLING = NULL;

Predicate<boost::shared_ptr<Entity> >* Turtle::BABY_ON_LAND_SELECTOR = new BabyOnLandSelector();

void Turtle::staticCtor()
{
    DATA_HOME_POS = SynchedEntityData::defineId(EntityType::TURTLE, EntityType::ANIMAL,
                                                EntityDataSerializers::BLOCK_POS);
    DATA_HAS_EGG = SynchedEntityData::defineId(EntityType::TURTLE, EntityType::ANIMAL, EntityDataSerializers::BOOLEAN);
    DATA_HAS_GIVEN_UP_ON_EGG = SynchedEntityData::defineId(EntityType::TURTLE, EntityType::ANIMAL,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_IS_LAYING_EGG = SynchedEntityData::defineId(EntityType::TURTLE, EntityType::ANIMAL,
                                                     EntityDataSerializers::BOOLEAN);
    DATA_TRAVEL_POS = SynchedEntityData::defineId(EntityType::TURTLE, EntityType::ANIMAL,
                                                  EntityDataSerializers::BLOCK_POS);
    DATA_GOING_HOME = SynchedEntityData::defineId(EntityType::TURTLE, EntityType::ANIMAL,
                                                  EntityDataSerializers::BOOLEAN);
    DATA_TRAVELLING = SynchedEntityData::defineId(EntityType::TURTLE, EntityType::ANIMAL,
                                                  EntityDataSerializers::BOOLEAN);
}

Turtle::Turtle(Level* level) : Animal(level)
{
    fjDerivedCtorCalls();
    setSize(1.2f, 0.4f);
    mMoveControl = new TurtleMoveControl(this);
    mSpawnableBlock = Blocks::sand;
    mLayEggTime = 0;
    mMaxUpStep = 1.0f;
}

Entity* Turtle::create(Level* level)
{
    return new Turtle(level);
}

void Turtle::setHomePos(const BlockPos& pos)
{
    mEntityData->set(DATA_HOME_POS, pos, true);
}

bool Turtle::hasEgg()
{
    return mEntityData->get(DATA_HAS_EGG);
}

void Turtle::setHasEgg(bool hasEgg)
{
    mEntityData->set(DATA_HAS_EGG, hasEgg, true);
}

bool Turtle::hasGivenUpOnEgg()
{
    return mEntityData->get(DATA_HAS_GIVEN_UP_ON_EGG);
}

BlockPos Turtle::getHomePos()
{
    return mEntityData->get(DATA_HOME_POS);
}

void Turtle::setGoingHome(bool goingHome)
{
    mEntityData->set(DATA_GOING_HOME, goingHome, true);
}

bool Turtle::closeToNextPos()
{
    Path* path = getNavigation()->getPath();
    if (path != NULL)
    {
        Node* target = path->getTarget();
        if (target != NULL && distanceToSqr(target->x, target->y, target->z) < 64.0)
        {
            return true;
        }
    }
    return false;
}

bool Turtle::isGoingHome()
{
    return mEntityData->get(DATA_GOING_HOME);
}

void Turtle::GiveUpOnEgg()
{
    mEntityData->set(DATA_HAS_GIVEN_UP_ON_EGG, true, true);
}

void Turtle::setIsLayingEgg(bool layingEgg)
{
    mEntityData->set(DATA_IS_LAYING_EGG, layingEgg, true);
}

bool Turtle::isLayingEgg()
{
    return mEntityData->get(DATA_IS_LAYING_EGG);
}

void Turtle::setTravelPos(const BlockPos& pos)
{
    mEntityData->set(DATA_TRAVEL_POS, pos, true);
}

void Turtle::setTravelling(bool travelling)
{
    mEntityData->set(DATA_TRAVELLING, travelling, true);
}

BlockPos Turtle::getTravelPos()
{
    return mEntityData->get(DATA_TRAVEL_POS);
}

void Turtle::registerGoals()
{
    mGoalSelector.addGoal(0, new TurtlePanicGoal(this, 1.2), true);
    mGoalSelector.addGoal(1, new TurtleGoHomeGoal(this, 1.0), true);
    mGoalSelector.addGoal(1, new TurtleLayEggGoal(this, 1.0, 4.0), true);
    mGoalSelector.addGoal(2, new TurtleBreedGoal(this, 1.0), true);
    static Item* temptItemsData[1] = {Item::byBlock(Blocks::sea_grass)};
    mGoalSelector.addGoal(
        3, new TemptGoal(this, 1.1, arrayWithLength<Item*>(temptItemsData, 1), false, arrayWithLength<int>()), true);
    mGoalSelector.addGoal(4, new TurtleGoToWaterGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new TurtleTravelGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(9, new TurtleRandomStrollGoal(this, 1.0, 80), true);
}

void Turtle::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(30.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
}

void Turtle::grewUp()
{
    AgableMob::grewUp();
    if (!mLevel->mIsClientSide)
        spawnAtLocation(Items::turtleShellPiece, 1);
}

void Turtle::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->define(DATA_HOME_POS, BlockPos::ZERO);
    mEntityData->define(DATA_HAS_EGG, false);
    mEntityData->define(DATA_HAS_GIVEN_UP_ON_EGG, false);
    mEntityData->define(DATA_IS_LAYING_EGG, false);
    mEntityData->define(DATA_TRAVEL_POS, BlockPos::ZERO);
    mEntityData->define(DATA_GOING_HOME, false);
    mEntityData->define(DATA_TRAVELLING, false);
}

PathNavigation* Turtle::createNavigation(Level* level)
{
    return new TurtlePathNavigation(this, level);
}

bool Turtle::generatesBubbles()
{
    return false;
}

void Turtle::tick()
{
    Mob::tick();
    if (isLayingEgg())
    {
        if (++mLayEggTime % 5 == 0)
        {
            BlockPos pos(getPos(0.0f));
            const BlockState* state = mLevel->getBlockState(pos.below());
            if (state->getBlock() == Blocks::sand)
            {
                mLevel->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getIdWithData(state));
            }
        }
    }
    else
    {
        mLayEggTime = 0;
    }
}

void Turtle::travel(float xxa, float yya, float zza)
{
    if (isEffectiveAi() && isInWater())
    {
        moveRelative(xxa, yya, zza, 0.1f);
        move(eMoverType_Self, mXd, mYd, mZd, false);
        mXd *= 0.9f;
        mYd *= 0.9f;
        mZd *= 0.9f;
        if (getTarget() == NULL && (!isGoingHome() || !(distanceToSqr(getHomePos()) < 400.0)))
        {
            mYd -= 0.005;
        }
    }
    else
    {
        LivingEntity::travel(xxa, yya, zza);
    }
}

boost::shared_ptr<AgableMob> Turtle::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<Turtle> child(new Turtle(mLevel));
    child->setHomePos(BlockPos(getPos(0.0f)));
    return child;
}

void Turtle::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putInt(L"HomePosX", getHomePos().x);
    tag->putInt(L"HomePosY", getHomePos().y);
    tag->putInt(L"HomePosZ", getHomePos().z);
    tag->putBoolean(L"HasEgg", hasEgg());
    tag->putBoolean(L"HasGivenUpOnEgg", hasGivenUpOnEgg());
    tag->putInt(L"TravelPosX", getTravelPos().x);
    tag->putInt(L"TravelPosY", getTravelPos().y);
    tag->putInt(L"TravelPosZ", getTravelPos().z);
}

void Turtle::readAdditionalSaveData(CompoundTag* tag)
{
    int homeX = tag->getInt(L"HomePosX");
    int homeY = tag->getInt(L"HomePosY");
    int homeZ = tag->getInt(L"HomePosZ");
    setHomePos(BlockPos(homeX, homeY, homeZ));
    Animal::readAdditionalSaveData(tag);
    setHasEgg(tag->getBoolean(L"HasEgg"));
    if (tag->getBoolean(L"HasGivenUpOnEgg"))
        GiveUpOnEgg();
    int travelX = tag->getInt(L"TravelPosX");
    int travelY = tag->getInt(L"TravelPosY");
    int travelZ = tag->getInt(L"TravelPosZ");
    setTravelPos(BlockPos(travelX, travelY, travelZ));
}

unsigned int Turtle::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (Item::getBlockId(holder.m_pItemInstance->getItem()) == BlockId::SEA_GRASS)
    {
        if (isBaby())
        {
            return IDS_TOOLTIPS_FEED;
        }
        if (!isInLove() && !isBreedCoolingDown() && !isBaby())
        {
            return IDS_TOOLTIPS_LOVEMODE;
        }
    }
    return AgableMob::GetInteractTooltip(holder);
}

bool Turtle::isTravelling()
{
    return mEntityData->get(DATA_TRAVELLING);
}

bool Turtle::breathesWater()
{
    return true;
}

MobGroupData* Turtle::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& dataString)
{
    setHomePos(BlockPos(mX, mY, mZ));
    setTravelPos(BlockPos::ZERO);
    return Mob::finalizeMobSpawn(difficulty, data, dataString);
}

bool Turtle::checkSpawnRules()
{
    return mBoundingBox->mMinY < mLevel->getSeaLevel() + 4 && Animal::checkSpawnRules();
}

bool Turtle::isPushedByWater()
{
    return false;
}

bool Turtle::isWaterMob()
{
    return true;
}

bool Turtle::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return Item::getBlockId(itemInstance->getItem()) == BlockId::SEA_GRASS;
}

float Turtle::getWalkTargetValue(const BlockPos& pos)
{
    if (mLevel->getBlockState(pos)->getMaterial() == Material::WATER)
        return 10.0f;
    return Animal::getWalkTargetValue(pos);
}

bool Turtle::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return false;
}

void Turtle::thunderHit(const LightningBolt* lightning)
{
    hurt(DamageSource::LIGHTNING_BOLT, FLT_MAX);
}

void Turtle::die(const DamageSource* source)
{
    LivingEntity::die(source);
    if (source == DamageSource::LIGHTNING_BOLT)
    {
        spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(Items::bowl, 1)), 0.0f);
    }
}

const SoundEvent* Turtle::getAmbientSound()
{
    return !isInWater() && mOnGround && !isBaby() ? SoundEvents::TURTLE_AMBIENT_LAND : Mob::getAmbientSound();
}

const SoundEvent* Turtle::getHurtSound(const DamageSource* source)
{
    return isBaby() ? SoundEvents::TURTLE_HURT_BABY : SoundEvents::TURTLE_HURT;
}

const SoundEvent* Turtle::getDeathSound()
{
    return isBaby() ? SoundEvents::TURTLE_DEATH_BABY : SoundEvents::TURTLE_DEATH;
}

void Turtle::playStepSound(const BlockPos& pos, Block* block)
{
    if (isBaby())
    {
        playSound(SoundEvents::TURTLE_SHAMBLE_BABY, 1.0f, 1.0f);
    }
    else
    {
        playSound(SoundEvents::TURTLE_SHAMBLE, 1.0f, 1.0f);
    }
}

const SoundEvent* Turtle::getSwimSound()
{
    return SoundEvents::TURTLE_SWIM;
}

int Turtle::getAmbientSoundInterval()
{
    return 200;
}

ResourceLocation* Turtle::getDefaultLootTable()
{
    return &BuiltInLootTables::SEA_TURTLE;
}

unsigned long long Turtle::GetType()
{
    return EntityType::TURTLE;
}
