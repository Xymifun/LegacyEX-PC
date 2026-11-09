#include "game/core/Parrot.hpp"
#include "game/entity/mob/EntityEvent.hpp"

#include <cfloat>
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <iterator>
#include <boost/unordered_map.hpp>

#include "game/core/DifficultyInstance.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/FlyingMoveControl.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/world/Level.hpp"

namespace
{
boost::unordered_map<unsigned long long, const SoundEvent*> gImitatedSounds;
}
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowMobGoal.hpp"
#include "game/entity/ai/FollowOwnerFlyingGoal.hpp"
#include "game/entity/ai/LandOnOwnersShoulderGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomFlyingGoal.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemId.hpp"

const EntityDataAccessor<int>* Parrot::DATA_VARIANT = NULL;
Item* Parrot::POISONOUS_FOOD = NULL;
arrayWithLength<Item*> Parrot::TAME_FOOD;
Predicate<boost::shared_ptr<Entity> >* Parrot::CAN_MIMIC = NULL;

Parrot::Parrot(Level* level) : ShoulderRidingEntity(level), mFlapping(1.0f)
{
    fjDerivedCtorCalls();
    _init();
    setSize(0.5f, 0.9f);
    mRecordPosition = NULL;
    delete mMoveControl;
    mMoveControl = new FlyingMoveControl(this);
}

Parrot::~Parrot()
{
    if (mRecordPosition != NULL)
    {
        delete mRecordPosition;
        mRecordPosition = NULL;
    }
}

Entity* Parrot::create(Level* level)
{
    return new Parrot(level);
}

bool Parrot::isFlying()
{
    return !mOnGround;
}

const SoundEvent* Parrot::getImitatedSound(unsigned long long type)
{
    boost::unordered_map<unsigned long long, const SoundEvent*>::const_iterator it = gImitatedSounds.find(type);
    return it != gImitatedSounds.end() ? it->second : SoundEvents::PARROT_AMBIENT;
}

float Parrot::getPitch(Random* random)
{
    return (random->nextFloat() - random->nextFloat()) * 0.2f + 1.5f;
}

bool Parrot::imitateNearbyMobs(Level* level, Entity* entity)
{
    if (entity->isSilent() || level->mRandom->nextInt(50) != 0)
        return false;

    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
        typeid(Mob), entity->mBoundingBox->grow(20.0), CAN_MIMIC, false);
    if (!entities->empty())
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(
            entities->at(level->mRandom->nextInt(static_cast<int>(entities->size()))));
        if (!mob->isSilent())
        {
            const SoundEvent* sound = getImitatedSound(mob->GetType());
            level->playSound(boost::shared_ptr<Player>(), entity->mX, entity->mY, entity->mZ, sound,
                             entity->getSoundSource(), 0.7f, getPitch(level->mRandom), 16.0f);
            delete entities;
            return true;
        }
    }
    delete entities;
    return false;
}

const SoundEvent* Parrot::getAmbient(Random* random)
{
    if (random->nextInt(1000) == 0)
    {
        boost::unordered_map<unsigned long long, const SoundEvent*>::const_iterator it = gImitatedSounds.begin();
        std::advance(it, random->nextInt((int)gImitatedSounds.size()));
        return getImitatedSound(it->first);
    }
    return SoundEvents::PARROT_AMBIENT;
}

void Parrot::playAmbientSound(Level* level, boost::shared_ptr<Entity> entity)
{
    if (entity->isSilent() || imitateNearbyMobs(level, entity.get()) || level->mRandom->nextInt(200) != 0)
        return;
    level->playSound(boost::shared_ptr<Player>(), entity->mX, entity->mY, entity->mZ, getAmbient(level->mRandom),
                     entity->getSoundSource(), 1.0f, getPitch(level->mRandom), 16.0f);
}

void Parrot::_init()
{
    mOFlap = 0.0f;
    mFlapSpeed = 0.0f;
    mFlap = 0.0f;
    mOFlapSpeed = 0.0f;
    mFlapping = 1.0f;
    mRecordPlayingNearby = false;
}

void Parrot::staticCtor()
{
    DATA_VARIANT = SynchedEntityData::defineId<int>(EntityType::PARROT, EntityType::TAMABLE_ANIMAL,
                                                    EntityDataSerializers::INT);
    CAN_MIMIC = new NotParrotPredicate();
    POISONOUS_FOOD = Items::cookie;
    static Item* tameFood[] = {Items::wheatSeeds, Items::melonSeeds, Items::pumpkinSeeds, Items::beetrootSeeds};
    TAME_FOOD = arrayWithLength<Item*>(tameFood, 4);
    gImitatedSounds[EntityType::BLAZE] = SoundEvents::PARROT_IMITATE_BLAZE;
    gImitatedSounds[EntityType::CAVE_SPIDER] = SoundEvents::PARROT_IMITATE_SPIDER;
    gImitatedSounds[EntityType::CREEPER] = SoundEvents::PARROT_IMITATE_CREEPER;
    gImitatedSounds[EntityType::ELDER_GUARDIAN] = SoundEvents::PARROT_IMITATE_ELDER_GUARDIAN;
    gImitatedSounds[EntityType::ENDER_DRAGON] = SoundEvents::PARROT_IMITATE_ENDERDRAGON;
    gImitatedSounds[EntityType::ENDER_MAN] = SoundEvents::PARROT_IMITATE_ENDERMAN;
    gImitatedSounds[EntityType::ENDERMITE] = SoundEvents::PARROT_IMITATE_ENDERMITE;
    gImitatedSounds[EntityType::EVOCATION_ILLAGER] = SoundEvents::PARROT_IMITATE_EVOCATION_ILLAGER;
    gImitatedSounds[EntityType::GHAST] = SoundEvents::PARROT_IMITATE_GHAST;
    gImitatedSounds[EntityType::HUSK] = SoundEvents::PARROT_IMITATE_HUSK;
    gImitatedSounds[EntityType::ILLUSION_ILLAGER] = SoundEvents::PARROT_IMITATE_ILLUSION_ILLAGER;
    gImitatedSounds[EntityType::MAGMA_CUBE] = SoundEvents::PARROT_IMITATE_MAGMACUBE;
    gImitatedSounds[EntityType::PIG_ZOMBIE] = SoundEvents::PARROT_IMITATE_ZOMBIE_PIGMAN;
    gImitatedSounds[EntityType::POLAR_BEAR] = SoundEvents::PARROT_IMITATE_POLAR_BEAR;
    gImitatedSounds[EntityType::SHULKER] = SoundEvents::PARROT_IMITATE_SHULKER;
    gImitatedSounds[EntityType::SILVERFISH] = SoundEvents::PARROT_IMITATE_SILVERFISH;
    gImitatedSounds[EntityType::SKELETON] = SoundEvents::PARROT_IMITATE_SKELETON;
    gImitatedSounds[EntityType::SLIME] = SoundEvents::PARROT_IMITATE_SLIME;
    gImitatedSounds[EntityType::SPIDER] = SoundEvents::PARROT_IMITATE_SPIDER;
    gImitatedSounds[EntityType::STRAY] = SoundEvents::PARROT_IMITATE_STRAY;
    gImitatedSounds[EntityType::VEX] = SoundEvents::PARROT_IMITATE_VEX;
    gImitatedSounds[EntityType::VINDICATION_ILLAGER] = SoundEvents::PARROT_IMITATE_VINDICATION_ILLAGER;
    gImitatedSounds[EntityType::WITCH] = SoundEvents::PARROT_IMITATE_WITCH;
    gImitatedSounds[EntityType::WITHER_BOSS] = SoundEvents::PARROT_IMITATE_WITHER;
    gImitatedSounds[EntityType::WITHER_SKELETON] = SoundEvents::PARROT_IMITATE_WITHER_SKELETON;
    gImitatedSounds[EntityType::WOLF] = SoundEvents::PARROT_IMITATE_WOLF;
    gImitatedSounds[EntityType::ZOMBIE] = SoundEvents::PARROT_IMITATE_ZOMBIE;
    gImitatedSounds[EntityType::ZOMBIE_VILLAGER] = SoundEvents::PARROT_IMITATE_ZOMBIE_VILLAGER;
}

void Parrot::setVariant(int variant)
{
    mEntityData->set(DATA_VARIANT, variant, true);
}

MobGroupData* Parrot::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                       const std::wstring& entityData)
{
    setVariant(mRandom->nextInt(5));
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

void Parrot::registerGoals()
{
    delete mSitGoal;
    mSitGoal = new SitGoal(this);
    mGoalSelector.addGoal(0, new PanicGoal(this, 1.25), true);
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(2, mSitGoal, false);
    mGoalSelector.addGoal(2, new FollowOwnerFlyingGoal(this, 1.0, 5.0f, 1.0f), true);
    mGoalSelector.addGoal(2, new WaterAvoidingRandomFlyingGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, new LandOnOwnersShoulderGoal(this), true);
    mGoalSelector.addGoal(3, new FollowMobGoal(this, 1.0, 3.0f, 7.0f), true);
}

void Parrot::registerAttributes()
{
    Mob::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::FLYING_SPEED);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::FLYING_SPEED)->setBaseValue(0.4f);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
}

PathNavigation* Parrot::createNavigation(Level* level)
{
    mFlyingNavigation = new FlyingPathNavigation(this, level);
    mFlyingNavigation->setCanOpenDoors(false);
    mFlyingNavigation->setCanFloat(true);
    mFlyingNavigation->setCanPassDoors(true);
    return mFlyingNavigation;
}

float Parrot::getEyeHeight()
{
    return static_cast<double>(mBbHeight * 0.6f);
}

void Parrot::calculateFlapping()
{
    mOFlap = mFlap;
    mOFlapSpeed = mFlapSpeed;
    mFlapSpeed += (mOnGround ? -1.0f : 4.0f) * 0.3f;
    mFlapSpeed = Mth::clamp(mFlapSpeed, 0.0f, 1.0f);
    if (!mOnGround && mFlapping < 1.0f)
        mFlapping = 1.0f;
    mFlapping *= 0.9f;
    if (!mOnGround && mYd < 0.0)
        mYd *= 0.6;
    mFlap += mFlapping * 2.0f;
}

void Parrot::aiStep()
{
    imitateNearbyMobs(mLevel, this);
    if (mRecordPosition == NULL || mRecordPosition->distSqr(mX, mY, mZ) > 12.0
        || mLevel->getBlockState(*mRecordPosition)->getBlock() != Blocks::jukebox)
    {
        mRecordPlayingNearby = false;
        mRecordPosition = NULL;
    }
    Animal::aiStep();
    calculateFlapping();
}

void Parrot::setRecordPlayingNearby(const BlockPos& pos, bool playing)
{
    delete mRecordPosition;
    mRecordPosition = new BlockPos(pos);
    mRecordPlayingNearby = playing;
}

bool Parrot::isPartyParrot()
{
    return mRecordPlayingNearby;
}

bool Parrot::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    bool feedingTameFood = false;
    Item* itemType = item->getItem();
    for (unsigned int i = 0; i < TAME_FOOD.length; ++i)
    {
        if (TAME_FOOD.data[i] == itemType)
        {
            feedingTameFood = true;
            break;
        }
    }

    if (!isTame() && feedingTameFood)
    {
        if (!player->mAbilities.mInstabuild)
            item->shrink(1);

        if (!isSilent())
        {
            mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::PARROT_EAT, getSoundSource(), 1.0f,
                              1.0f + (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f, 16.0f);
        }

        if (!mLevel->mIsClientSide)
        {
            if (mRandom->nextInt(10) == 0)
            {
                tame(player);
                spawnTamingParticles(true);
                mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_SUCCEEDED, 0);
            }
            else
            {
                spawnTamingParticles(false);
                mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TAMING_FAILED, 0);
            }
        }

        return true;
    }
    else if (itemType == POISONOUS_FOOD)
    {
        if (!player->mAbilities.mInstabuild)
            item->shrink(1);

        addEffect(new MobEffectInstance(MobEffects::POISON, 900), boost::shared_ptr<Entity>());

        if (player->isCreative() || !isInvulnerable())
            hurt(DamageSource::CreatePlayerAttack(player), FLT_MAX);

        return true;
    }
    else if (!mLevel->mIsClientSide && !isFlying() && isTame() && isOwnedBy(player))
    {
        bool renaming = false;
        if (Item::getId(itemType) == ItemId::NAME_TAG && !isPassenger() && item->hasCustomHoverName()
            && (!hasCustomName() || !(item->getHoverName() == getCustomName())))
            renaming = true;

        if (!renaming && !isLeashedToPlayer(player))
        {
            if (item->isEmpty() || itemType != Items::lead)
                mSitGoal->wantToSit(!isSitting());
        }
    }

    return Animal::mobInteract(player, hand);
}

bool Parrot::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return false;
}

bool Parrot::checkSpawnRules()
{
    int x = Mth::floor(mX);
    int y = Mth::floor(mBoundingBox->mMinY);
    int z = Mth::floor(mZ);
    BlockPos pos(x, y, z);
    Block* block = mLevel->getBlockState(pos.below())->getBlock();
    return (block != NULL && dynamic_cast<LeafBlock*>(block) != NULL) || block == Blocks::grass
           || (block != NULL && dynamic_cast<LogBlock*>(block) != NULL)
           || (block == Blocks::air && mLevel->getDaytimeRawBrightness(pos) > 8 && Animal::checkSpawnRules());
}

void Parrot::causeFallDamage(float, float)
{
}

bool Parrot::isTameFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    Item* item = itemInstance->getItem();
    for (unsigned int i = 0; i < TAME_FOOD.length; ++i)
    {
        if (TAME_FOOD.data[i] == item)
            return true;
    }
    return false;
}

unsigned int Parrot::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (isLeashedToPlayer(holder.m_pPlayer))
        return IDS_TOOLTIPS_UNLEASH;

    if (holder.m_pItemInstance->getItem() == POISONOUS_FOOD)
        return IDS_NONE;

    if (!isPassenger())
    {
        if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::NAME_TAG)
        {
            if (holder.m_pItemInstance->hasCustomHoverName())
            {
                if (!hasCustomName() || !(holder.m_pItemInstance->getHoverName() == getCustomName()))
                    return IDS_TOOLTIPS_NAME;
            }

            if (isOwnedBy(holder.m_pPlayer))
            {
                if (isSitting())
                    return IDS_TOOLTIPS_FOLLOWME;
                if (!isFlying())
                    return IDS_TOOLTIPS_SIT;
            }
            return IDS_NONE;
        }
    }

    if (!isTame())
    {
        if (isTameFood(holder.m_pItemInstance) && getPassengers()->empty())
            return IDS_TOOLTIPS_TAME;
        return IDS_NONE;
    }

    if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::LEAD && !isLeashed())
        return IDS_TOOLTIPS_LEASH;

    if (isOwnedBy(holder.m_pPlayer))
    {
        if (isSitting())
            return IDS_TOOLTIPS_FOLLOWME;
        if (!isFlying())
            return IDS_TOOLTIPS_SIT;
    }
    return IDS_NONE;
}

void Parrot::checkFallDamage(double, bool, Block*, const BlockPos&)
{
}

bool Parrot::canMate(const boost::shared_ptr<Animal>&)
{
    return false;
}

boost::shared_ptr<AgableMob> Parrot::getBreedOffspring(boost::shared_ptr<AgableMob>)
{
    return boost::shared_ptr<AgableMob>();
}

bool Parrot::doHurtTarget(boost::shared_ptr<Entity> target)
{
    EntityDamageSource* damageSource = DamageSource::CreateMobAttack(
        boost::static_pointer_cast<LivingEntity>(shared_from_this()));
    bool result = target->hurt(damageSource, 3.0f);
    delete damageSource;
    return result;
}

const SoundEvent* Parrot::getAmbientSound()
{
    return getAmbient(mRandom);
}

const SoundEvent* Parrot::getHurtSound(const DamageSource*)
{
    return SoundEvents::PARROT_HURT;
}

const SoundEvent* Parrot::getDeathSound()
{
    return SoundEvents::PARROT_DEATH;
}

void Parrot::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::PARROT_STEP, 0.15000000596046448f, 1.0f);
}

float Parrot::playFlySound(float volume)
{
    playSound(SoundEvents::PARROT_FLY, 0.15f, 1.0f);
    return volume + mFlapSpeed * 0.5f;
}

bool Parrot::makeFlySound()
{
    return true;
}

float Parrot::getVoicePitch()
{
    return getPitch(mRandom);
}

SoundSource::ESoundSource Parrot::getSoundSource()
{
    return SoundSource::eSoundSource_Neutral;
}

bool Parrot::isPushable(boost::shared_ptr<Entity>)
{
    return true;
}

void Parrot::doPush(boost::shared_ptr<Entity> entity)
{
    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType());
    if (!isPlayer)
        LivingEntity::doPush(entity);
}

bool Parrot::hurt(const DamageSource* source, float damage)
{
    if (isTame())
    {
        boost::shared_ptr<Entity> direct = source->getDirectEntity();
        if (direct)
        {
            bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, direct->GetType());
            if (isPlayer)
            {
                boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(direct);
                if (!player->canHarmPlayer(getOwnerUUID()))
                    return false;
            }
        }
    }

    if (isInvulnerableTo(source))
        return false;
    if (mSitGoal != NULL)
        mSitGoal->wantToSit(false);
    return Animal::hurt(source, damage);
}

int Parrot::getVariant()
{
    return Mth::clamp(mEntityData->get(DATA_VARIANT), 0, 4);
}

void Parrot::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
    mEntityData->createDateItem(DATA_VARIANT, 0);
}

void Parrot::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
}

void Parrot::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    setVariant(tag->getInt(L"Variant"));
}

ResourceLocation* Parrot::getDefaultLootTable()
{
    return &BuiltInLootTables::PARROT;
}

unsigned long long Parrot::GetType()
{
    return EntityType::PARROT;
}

bool Parrot::NotParrotPredicate::apply(boost::shared_ptr<Entity> entity) const
{
    unsigned long long type = entity->GetType();
    return entity != NULL && gImitatedSounds.find(type) != gImitatedSounds.end();
}
