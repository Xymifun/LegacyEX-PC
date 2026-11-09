#include "game/util/Rabbit.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/CarrotBlock.hpp"
#include "game/world/biome/DesertBiome.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Path.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"

static const EntityDataAccessor<int>* DATA_RABBIT_TYPE = NULL;

Rabbit::EvilRabbitAttackGoal::EvilRabbitAttackGoal(Rabbit* rabbit) : MeleeAttackGoal(rabbit, 1.4, true)
{
}

double Rabbit::EvilRabbitAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    return target->mBbWidth + 4.0f;
}

Rabbit::RabbitAvoidEntityGoal::RabbitAvoidEntityGoal(Rabbit* rabbit, const std::type_info& avoidType, float maxDistance,
                                                     double walkSpeedModifier, double sprintSpeedModifier)
    : AvoidEntityGoal(rabbit, avoidType, maxDistance, walkSpeedModifier, sprintSpeedModifier),
      mRabbit(rabbit)
{
}

bool Rabbit::RabbitAvoidEntityGoal::canUse()
{
    return mRabbit->getRabbitType() != 99 && AvoidEntityGoal::canUse();
}

Rabbit::RabbitGroupData::RabbitGroupData(int type) : mRabbitType(type)
{
}

Rabbit::RabbitGroupData::~RabbitGroupData()
{
}

Rabbit::RabbitJumpControl::RabbitJumpControl(Rabbit* rabbit) : JumpControl(rabbit), mRabbit(rabbit), mCanJump(false)
{
}

Rabbit::RabbitJumpControl::~RabbitJumpControl()
{
}

void Rabbit::RabbitJumpControl::setCanJump(bool canJump)
{
    mCanJump = canJump;
}

unsigned char Rabbit::RabbitJumpControl::wantJump()
{
    return mJump ? 1 : 0;
}

unsigned char Rabbit::RabbitJumpControl::canJump()
{
    return mCanJump ? 1 : 0;
}

void Rabbit::RabbitJumpControl::tick()
{
    if (mJump)
    {
        mRabbit->startJumping();
        mJump = false;
    }
}

Rabbit::RabbitMoveControl::RabbitMoveControl(Rabbit* rabbit) : MoveControl(rabbit), mRabbit(rabbit), mSpeedModifier(0.0)
{
}

Rabbit::RabbitMoveControl::~RabbitMoveControl()
{
}

void Rabbit::RabbitMoveControl::tick()
{
    if (mRabbit->mOnGround && !mRabbit->mJumping && !static_cast<RabbitJumpControl*>(mRabbit->mJumpControl)->wantJump())
    {
        mRabbit->setSpeedModifier(0.0);
    }
    else if (hasWanted())
    {
        mRabbit->setSpeedModifier(mSpeedModifier);
    }
    MoveControl::tick();
}

void Rabbit::RabbitMoveControl::setWantedPosition(double x, double y, double z, double speedModifier)
{
    if (mRabbit->isInWater())
        speedModifier = 1.5;
    MoveControl::setWantedPosition(x, y, z, speedModifier);
    if (speedModifier > 0.0)
        mSpeedModifier = speedModifier;
}

Rabbit::RabbitPanicGoal::RabbitPanicGoal(Rabbit* rabbit, double speedModifier)
    : PanicGoal(rabbit, speedModifier),
      mRabbit(rabbit)
{
}

Rabbit::RabbitPanicGoal::~RabbitPanicGoal()
{
}

void Rabbit::RabbitPanicGoal::tick()
{
    PanicGoal::tick();
    mRabbit->setSpeedModifier(mSpeedModifier);
}

Rabbit::RaidGardenGoal::RaidGardenGoal(Rabbit* rabbit) : MoveToBlockGoal(rabbit, 0.7f, 16, 1, 1.0), mRabbit(rabbit)
{
    mWantsToRaid = mCanRaid = false;
}

Rabbit::RaidGardenGoal::~RaidGardenGoal()
{
}

bool Rabbit::RaidGardenGoal::canUse()
{
    if (mNextStartTick < 1)
    {
        if (!mRabbit->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
            return false;
        mCanRaid = false;
        mWantsToRaid = true;
    }
    return MoveToBlockGoal::canUse();
}

bool Rabbit::RaidGardenGoal::canContinueToUse()
{
    return mCanRaid && MoveToBlockGoal::canContinueToUse();
}

void Rabbit::RaidGardenGoal::tick()
{
    MoveToBlockGoal::tick();
    mRabbit->getLookControl()->setLookAt(mBlockPos.x + 0.5, mBlockPos.y + 1, mBlockPos.z + 0.5, 10.0f,
                                         static_cast<float>(mRabbit->getMaxHeadXRot()));
    if (isReachedTarget())
    {
        Level* level = mRabbit->mLevel;
        BlockPos cropPos = mBlockPos.above();
        const BlockState* state = level->getBlockState(cropPos);
        Block* block = state->getBlock();
        if (mCanRaid && dynamic_cast<CarrotBlock*>(block) != NULL)
        {
            int age = state->getValue(CropBlock::AGE);
            if (age == 0)
            {
                level->setBlock(cropPos, Blocks::air->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                level->destroyBlock(cropPos, true);
                mRabbit->eatCarrot();
            }
            else
            {
                level->setBlock(cropPos, state->setValue(CropBlock::AGE, age - 1), Block::UPDATE_CLIENTS, false);
                level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, cropPos, Block::getIdWithData(state));
                mRabbit->eatCarrot();
            }
        }
        mCanRaid = false;
        mNextStartTick = 10;
    }
}

bool Rabbit::RaidGardenGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    BlockPos farmPos = pos;
    Block* block = level->getBlockState(farmPos)->getBlock();
    if (block == Blocks::farmland && mWantsToRaid && !mCanRaid)
    {
        farmPos = farmPos.above();
        const BlockState* state = level->getBlockState(farmPos);
        block = state->getBlock();
        if (dynamic_cast<CarrotBlock*>(block) != NULL && state->getValue(CropBlock::AGE) == 7)
        {
            mCanRaid = true;
            return true;
        }
    }
    return false;
}

Rabbit::Rabbit(Level* level) : Animal(level)
{
    fjDerivedCtorCalls();
    mMoreCarrotTicks = 0;
    mSpawnBlocks[0] = Blocks::grass;
    mJumpTicks = 0;
    mJumpDuration = 0;
    mSpawnBlocks[2] = Blocks::snow_layer;
    mSpawnBlocks[3] = Blocks::sand;
    mSpawnBlocks[1] = Blocks::snow;
    mJumping = false;
    mJumpDelayTicks = 0;
    setSize(0.4f, 0.5f);
    delete mJumpControl;
    mJumpControl = new RabbitJumpControl(this);
    delete mMoveControl;
    mMoveControl = new RabbitMoveControl(this);
    setSpeedModifier(0.0);
}

Entity* Rabbit::create(Level* level)
{
    return new Rabbit(level);
}

void Rabbit::staticCtor()
{
    DATA_RABBIT_TYPE = SynchedEntityData::defineId<int>(EntityType::RABBIT, EntityType::ANIMAL,
                                                        EntityDataSerializers::INT);
}

void Rabbit::setSpeedModifier(double speedModifier)
{
    getNavigation()->setSpeedModifier(speedModifier);
    mMoveControl->setWantedPosition(mMoveControl->getWantedX(), mMoveControl->getWantedY(), mMoveControl->getWantedZ(),
                                    speedModifier);
}

void Rabbit::registerGoals()
{
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new RabbitPanicGoal(this, 2.2), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 0.8), true);

    static Item* carrotItems[] = {Items::carrot};
    mGoalSelector.addGoal(
        3, new TemptGoal(this, 1.0, arrayWithLength<Item*>(carrotItems, 1), false, arrayWithLength<int>()), true);
    static Item* goldenCarrotItems[] = {Items::goldenCarrot};
    mGoalSelector.addGoal(
        3, new TemptGoal(this, 1.0, arrayWithLength<Item*>(goldenCarrotItems, 1), false, arrayWithLength<int>()), true);
    static Item* flowerItems[] = {Item::byBlock(Blocks::yellow_flower)};
    mGoalSelector.addGoal(
        3, new TemptGoal(this, 1.0, arrayWithLength<Item*>(flowerItems, 1), false, arrayWithLength<int>()), true);

    mGoalSelector.addGoal(4, new RabbitAvoidEntityGoal(this, typeid(Player), 8.0f, 2.2, 2.2), true);
    mGoalSelector.addGoal(4, new RabbitAvoidEntityGoal(this, typeid(Wolf), 10.0f, 2.2, 2.2), true);
    mGoalSelector.addGoal(4, new RabbitAvoidEntityGoal(this, typeid(Monster), 4.0f, 2.2, 2.2), true);
    mGoalSelector.addGoal(5, new RaidGardenGoal(this), true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(11, new LookAtPlayerGoal(this, typeid(Player), 10.0f), true);
}

float Rabbit::getJumpPower()
{
    if (mHorizontalCollision || (mMoveControl->hasWanted() && mMoveControl->getWantedY() > mY + 0.5))
        return 0.5f;
    Path* path = mNavigation->getPath();
    if (path != NULL && path->getIndex() < path->getSize())
    {
        Vec3* pos = path->currentPos(shared_from_this());
        if (pos->y > mY + 0.5)
            return 0.5f;
    }
    return mMoveControl->getSpeedModifier() <= 0.6 ? 0.2f : 0.3f;
}

void Rabbit::jumpFromGround()
{
    LivingEntity::jumpFromGround();
    if (mMoveControl->getSpeedModifier() > 0.0 && mXd * mXd + mZd * mZd < 0.010000000000000002)
        moveRelative(0.0f, 0.0f, 1.0f, 0.1f);
    if (!mLevel->mIsClientSide)
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::JUMP, 0);
}

float Rabbit::getJumpCompletion(float partialTick)
{
    if (mJumpDuration == 0)
        return 0.0f;
    return (static_cast<float>(mJumpTicks) + partialTick) / static_cast<float>(mJumpDuration);
}

const SoundEvent* Rabbit::getJumpSound()
{
    return SoundEvents::RABBIT_JUMP;
}

void Rabbit::setJumping(bool jumping)
{
    LivingEntity::setJumping(jumping);
    if (jumping)
    {
        playSound(getJumpSound(), getSoundVolume(),
                  ((mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f) * 0.8f);
    }
}

void Rabbit::startJumping()
{
    setJumping(true);
    mJumpDuration = 10;
    mJumpTicks = 0;
}

bool Rabbit::checkSpawnRules()
{
    int x = Mth::floor(mX);
    int y = Mth::floor(mBoundingBox->mMinY);
    int z = Mth::floor(mZ);
    BlockPos pos(x, y, z);
    bool spawnBlock = false;
    for (int i = 0; i < SPAWN_BLOCK_COUNT; ++i)
    {
        if (mLevel->getBlockState(pos.below())->getBlock() == mSpawnBlocks[i])
        {
            spawnBlock = true;
            break;
        }
    }
    return spawnBlock && mLevel->getDaytimeRawBrightness(pos) > 8 && PathfinderMob::checkSpawnRules();
}

void Rabbit::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->createDateItem(DATA_RABBIT_TYPE, 0);
}

void Rabbit::setLandingDelay()
{
    mJumpDelayTicks = mMoveControl->getSpeedModifier() < 2.2 ? 10 : 1;
}

void Rabbit::disableJumpControl()
{
    static_cast<RabbitJumpControl*>(mJumpControl)->setCanJump(false);
}

void Rabbit::checkLandingDelay()
{
    setLandingDelay();
    disableJumpControl();
}

int Rabbit::getRabbitType()
{
    return mEntityData->get(DATA_RABBIT_TYPE);
}

void Rabbit::facePoint(double x, double z)
{
    mYRot = static_cast<float>((std::atan2(z - mZ, x - mX) * 180.0) / static_cast<double>(3.1415927f)) - 90.0f;
}

void Rabbit::enableJumpControl()
{
    static_cast<RabbitJumpControl*>(mJumpControl)->setCanJump(true);
}

void Rabbit::customServerAiStep()
{
    if (mJumpDelayTicks > 0)
        --mJumpDelayTicks;
    if (mMoreCarrotTicks > 0)
    {
        mMoreCarrotTicks -= mRandom->nextInt(3);
        if (mMoreCarrotTicks < 0)
            mMoreCarrotTicks = 0;
    }

    if (mOnGround)
    {
        if (!mWasOnGround)
        {
            setJumping(false);
            checkLandingDelay();
        }

        if (getRabbitType() == 99 && mJumpDelayTicks == 0)
        {
            boost::shared_ptr<LivingEntity> target = getTarget();
            if (target != NULL && distanceToSqr(target) < 16.0)
            {
                facePoint(target->mX, target->mZ);
                mMoveControl->setWantedPosition(target->mX, target->mY, target->mZ, mMoveControl->getSpeedModifier());
                startJumping();
                mWasOnGround = true;
            }
        }

        RabbitJumpControl* jumpControl = static_cast<RabbitJumpControl*>(mJumpControl);
        if (!jumpControl->wantJump())
        {
            if (mMoveControl->hasWanted() && mJumpDelayTicks == 0)
            {
                Path* path = mNavigation->getPath();
                Vec3* pos = Vec3::newTemp(mMoveControl->getWantedX(), mMoveControl->getWantedY(),
                                          mMoveControl->getWantedZ());
                if (path != NULL && path->getIndex() < path->getSize())
                    pos = path->currentPos(shared_from_this());
                facePoint(pos->x, pos->z);
                startJumping();
            }
        }
        else if (!jumpControl->canJump())
        {
            enableJumpControl();
        }
    }
    mWasOnGround = mOnGround;
}

void Rabbit::updateSprintingState()
{
}

void Rabbit::aiStep()
{
    Animal::aiStep();
    if (mJumpTicks != mJumpDuration)
        ++mJumpTicks;
    else if (mJumpDuration != 0)
    {
        mJumpTicks = 0;
        mJumpDuration = 0;
        setJumping(false);
    }
}

void Rabbit::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(3.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
}

void Rabbit::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::RABBIT);
}

void Rabbit::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putInt(L"RabbitType", getRabbitType());
    tag->putInt(L"MoreCarrotTicks", mMoreCarrotTicks);
}

void Rabbit::setRabbitType(int type)
{
    if (type == 99)
    {
        mGoalSelector.addGoal(4, new EvilRabbitAttackGoal(this), true);
        mTargetSelector.addGoal(1, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
        mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
        mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Wolf), true, false, false), true);
        if (!hasCustomName())
        {
            setCustomName(L"entity.KillerBunny.name", L"", false);
        }
    }
    mEntityData->set(DATA_RABBIT_TYPE, type, true);
}

void Rabbit::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    setRabbitType(tag->getInt(L"RabbitType"));
    mMoreCarrotTicks = tag->getInt(L"MoreCarrotTicks");
}

const SoundEvent* Rabbit::getAmbientSound()
{
    return SoundEvents::RABBIT_AMBIENT;
}

const SoundEvent* Rabbit::getHurtSound(const DamageSource*)
{
    return SoundEvents::RABBIT_HURT;
}

const SoundEvent* Rabbit::getDeathSound()
{
    return SoundEvents::RABBIT_DEATH;
}

bool Rabbit::doHurtTarget(boost::shared_ptr<Entity> target)
{
    DamageSource* damageSource = DamageSource::CreateMobAttack(
        boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
    if (getRabbitType() == 99)
    {
        playSound(SoundEvents::RABBIT_ATTACK, 1.0f, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
        target->hurt(damageSource, 8.0f);
    }
    bool hurt = target->hurt(damageSource, 3.0f);
    delete damageSource;
    return hurt;
}

SoundSource::ESoundSource Rabbit::getSoundSource()
{
    return getRabbitType() == 99 ? SoundSource::eSoundSource_Hostile : SoundSource::eSoundSource_Neutral;
}

bool Rabbit::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    return Animal::hurt(source, damage);
}

ResourceLocation* Rabbit::getDefaultLootTable()
{
    return &BuiltInLootTables::RABBIT;
}

bool Rabbit::isTemptingItem(Item* item)
{
    return item == Items::carrot || item == Items::goldenCarrot || item == Item::byBlock(Blocks::yellow_flower);
}

int Rabbit::getRandomRabbitType()
{
    Biome* biome = mLevel->getBiome(BlockPos(shared_from_this()));
    int roll = mRandom->nextInt(100);
    if (biome->isSnowCovered())
        return roll < 80 ? 1 : 3;
    if (dynamic_cast<DesertBiome*>(biome) != NULL)
        return 4;
    if (roll < 50)
        return 0;
    return roll < 90 ? 5 : 2;
}

boost::shared_ptr<AgableMob> Rabbit::getBreedOffspring(boost::shared_ptr<AgableMob> parent)
{
    boost::shared_ptr<Rabbit> child(new Rabbit(mLevel));
    int type = getRandomRabbitType();
    if (mRandom->nextInt(20) != 0)
    {
        bool parentIsRabbit = parent->GetType() == EntityType::RABBIT;
        if (parentIsRabbit && mRandom->nextBoolean())
        {
            boost::shared_ptr<Rabbit> other = boost::static_pointer_cast<Rabbit>(parent);
            type = other->getRabbitType();
        }
        else
        {
            type = getRabbitType();
        }
    }
    child->setRabbitType(type);
    return child;
}

bool Rabbit::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return isTemptingItem(itemInstance->getItem());
}

MobGroupData* Rabbit::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& entityData)
{
    MobGroupData* groupData = Mob::finalizeMobSpawn(difficulty, data, L"");
    int type = getRandomRabbitType();
    bool isBaby = false;
    if (dynamic_cast<RabbitGroupData*>(groupData) != NULL)
    {
        type = static_cast<RabbitGroupData*>(groupData)->mRabbitType;
        isBaby = true;
    }
    else
    {
        groupData = new RabbitGroupData(type);
    }
    setRabbitType(type);
    if (isBaby)
        setAge(-24000);
    return groupData;
}

void Rabbit::eatCarrot()
{
    CropBlock* carrots = static_cast<CropBlock*>(Blocks::carrots);
    int particleData = Block::getIdWithData(carrots->getStateForAge(carrots->getMaxAge()));
    mLevel->addParticle(PARTICLE_BLOCK_DUST, mX + (mRandom->nextFloat() * mBbWidth * 2.0f) - mBbWidth,
                        mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                        mZ + (mRandom->nextFloat() * mBbWidth * 2.0f) - mBbWidth, 0.0, 0.0, 0.0,
                        arrayWithLength<int>(&particleData, 1));
    mMoreCarrotTicks = 40;
}

void Rabbit::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::JUMP)
    {
        doSprintParticleEffect();
        mJumpDuration = 10;
        mJumpTicks = 0;
        return;
    }
    Animal::handleEntityEvent(event, data);
}

unsigned long long Rabbit::GetType()
{
    return EntityType::RABBIT;
}

bool Rabbit::wantsMoreFood()
{
    return mMoreCarrotTicks == 0;
}
