#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/Difficulty.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Wolf.hpp"
#include "game/core/StringIds.hpp"
#include "game/item/ItemId.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/BegGoal.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowOwnerGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LeapAtTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/OwnerHurtByTargetGoal.hpp"
#include "game/entity/ai/OwnerHurtTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/Inventory.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Llama.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/PreySelector.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/ai/NonTameRandomTargetGoal.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/core/DyeColor.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/FoodItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/world/Level.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/core/Mth.hpp"

static Predicate<boost::shared_ptr<Entity> >* PREY_SELECTOR = new PreySelector();
static const int WOLF_SPAWN_EGG_AUX = 95;
static const int WOLF_ANGRY_FLAG = 2;
static const int WOLF_COLLAR_COLOR_MASK = 15;
static const float WOLF_PI = 3.1415927f;

const EntityDataAccessor<float>* Wolf::DATA_HEALTH_ID = NULL;
const EntityDataAccessor<bool>* Wolf::DATA_INTERESTED = NULL;
const EntityDataAccessor<int>* Wolf::DATA_COLLAR_COLOR = NULL;
Wolf::Wolf(Level* level) : TamableAnimal(level)
{
    fjDerivedCtorCalls();
    mInterestedAngle = 0.0f;
    mInterestedAngleO = 0.0f;
    mIsWet = false;
    mIsShaking = false;
    mShakeAnim = 0.0f;
    mShakeAnimO = 0.0f;
    setSize(0.6f, 0.85f);
}
Entity* Wolf::create(Level* level)
{
    return new Wolf(level);
}
void Wolf::staticCtor()
{
    DATA_HEALTH_ID = SynchedEntityData::defineId<float>(EntityType::WOLF, EntityType::TAMABLE_ANIMAL,
                                                        EntityDataSerializers::FLOAT);
    DATA_INTERESTED = SynchedEntityData::defineId<bool>(EntityType::WOLF, EntityType::TAMABLE_ANIMAL,
                                                        EntityDataSerializers::BOOLEAN);
    DATA_COLLAR_COLOR = SynchedEntityData::defineId<int>(EntityType::WOLF, EntityType::TAMABLE_ANIMAL,
                                                         EntityDataSerializers::INT);
}
void Wolf::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::WOLF);
}
void Wolf::registerGoals()
{
    delete mSitGoal;
    mSitGoal = new SitGoal(this);
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, mSitGoal, false);
    mGoalSelector.addGoal(3, new WolfAvoidEntityGoal(this, typeid(Llama), 24.0f, 1.5, 1.5), true);
    mGoalSelector.addGoal(4, new LeapAtTargetGoal(this, 0.4f), true);
    mGoalSelector.addGoal(5, new MeleeAttackGoal(this, 1.0, true), true);
    mGoalSelector.addGoal(6, new FollowOwnerGoal(this, 1.0, 10.0f, 2.0f), true);
    mGoalSelector.addGoal(7, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(9, new BegGoal(this, 8.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(10, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new OwnerHurtByTargetGoal(this), true);
    mTargetSelector.addGoal(2, new OwnerHurtTargetGoal(this), true);
    mTargetSelector.addGoal(3, new HurtByTargetGoal(this, true, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(4, new NonTameRandomTargetGoal(this, typeid(Animal), true, PREY_SELECTOR, false), true);
    mTargetSelector.addGoal(
        4, new NonTameRandomTargetGoal(this, typeid(Turtle), false, Turtle::BABY_ON_LAND_SELECTOR, false), true);
    mTargetSelector.addGoal(5, new NearestAttackableTargetGoal(this, typeid(AbstractSkeleton), true, false, false),
                            true);
}
void Wolf::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
    if (isTame())
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(20.0);
    else
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(2.0);
}
void Wolf::setAngry(bool angry)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    if (angry)
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags | WOLF_ANGRY_FLAG), true);
    else
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags & ~WOLF_ANGRY_FLAG), true);
}
void Wolf::setTarget(boost::shared_ptr<LivingEntity> target)
{
    Mob::setTarget(target);
    if (target == NULL)
        setAngry(false);
    else if (!isTame())
        setAngry(true);
}
void Wolf::customServerAiStep()
{
    mEntityData->set(DATA_HEALTH_ID, getHealth(), true);
}
void Wolf::defineSynchedData()
{
    TamableAnimal::defineSynchedData();
    mEntityData->define(DATA_HEALTH_ID, getHealth());
    mEntityData->define(DATA_INTERESTED, false);
    mEntityData->define(DATA_COLLAR_COLOR, DyeColor::RED->getItemData());
}
void Wolf::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::WOLF_STEP, 0.15f, 1.0f);
}
bool Wolf::isAngry()
{
    return (mEntityData->get(DATA_FLAGS) & WOLF_ANGRY_FLAG) != 0;
}
const DyeColor* Wolf::getCollarColor()
{
    return DyeColor::byItemData(mEntityData->get(DATA_COLLAR_COLOR) & WOLF_COLLAR_COLOR_MASK);
}
void Wolf::addAdditonalSaveData(CompoundTag* tag)
{
    TamableAnimal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Angry", isAngry());
    tag->putByte(L"CollarColor", static_cast<unsigned char>(getCollarColor()->getItemData()));
}
void Wolf::setCollarColor(const DyeColor* color)
{
    mEntityData->set(DATA_COLLAR_COLOR, color->getItemData() & WOLF_COLLAR_COLOR_MASK, true);
}
void Wolf::readAdditionalSaveData(CompoundTag* tag)
{
    TamableAnimal::readAdditionalSaveData(tag);
    setAngry(tag->getBoolean(L"Angry"));
    if (tag->contains(L"CollarColor"))
        setCollarColor(DyeColor::byItemData(tag->getByte(L"CollarColor")));
}
const SoundEvent* Wolf::getAmbientSound()
{
    if (isAngry())
        return SoundEvents::WOLF_GROWL;
    if (mRandom->nextInt(3) == 0)
    {
        if (isTame() && mEntityData->get(DATA_HEALTH_ID) < 10.0f)
            return SoundEvents::WOLF_WHINE;
        return SoundEvents::WOLF_PANT;
    }
    return SoundEvents::WOLF_AMBIENT;
}
const SoundEvent* Wolf::getHurtSound(const DamageSource*)
{
    return SoundEvents::WOLF_HURT;
}
const SoundEvent* Wolf::getDeathSound()
{
    return SoundEvents::WOLF_DEATH;
}
float Wolf::getSoundVolume()
{
    return 0.4f;
}
ResourceLocation* Wolf::getDefaultLootTable()
{
    return &BuiltInLootTables::WOLF;
}
void Wolf::aiStep()
{
    Animal::aiStep();
    if (!mLevel->mIsClientSide && mIsWet && !mIsShaking && !isPathFinding() && mOnGround)
    {
        mIsShaking = true;
        mShakeAnim = 0.0f;
        mShakeAnimO = 0.0f;
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::SHAKE_WETNESS, 0);
    }
    if (!mLevel->mIsClientSide && (getTarget() == NULL || !getTarget()->isAlive()) && isAngry())
        setAngry(false);
}
bool Wolf::isInterested()
{
    return mEntityData->get(DATA_INTERESTED);
}
void Wolf::tick()
{
    PIXBeginNamedEvent(0.0f, "Wolf tick");
    Mob::tick();

    mInterestedAngleO = mInterestedAngle;
    if (isInterested())
    {
        mInterestedAngle += (1.0f - mInterestedAngle) * 0.4f;
    }
    else
    {
        mInterestedAngle += (0.0f - mInterestedAngle) * 0.4f;
    }

    if (isInWaterRainOrBubble())
    {
        mIsWet = true;
        mIsShaking = false;
        mShakeAnim = 0.0f;
        mShakeAnimO = 0.0f;
    }
    else if (mIsShaking)
    {
        if (mShakeAnim == 0.0f)
        {
            playSound(SoundEvents::WOLF_SHAKE, getSoundVolume(),
                      (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
        }

        const float previousShakeAnim = mShakeAnim;
        const float currentShakeAnim = previousShakeAnim + 0.05f;
        mShakeAnim = currentShakeAnim;

        if (previousShakeAnim >= 2.0f)
        {
            mIsWet = false;
            mIsShaking = false;
            mShakeAnimO = 0.0f;
            mShakeAnim = 0.0f;
        }
        else
        {
            mShakeAnimO = previousShakeAnim;
            if (currentShakeAnim > 0.4f)
            {
                float y = (float)mBoundingBox->mMinY;
                int count = (int)(Mth::sin((currentShakeAnim - 0.4f) * WOLF_PI) * 7.0f);

                for (int i = 0; i < count; ++i)
                {
                    float xOffset = (mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                    float zOffset = (mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth * 0.5f;
                    mLevel->addParticle(PARTICLE_SPLASH, mX + xOffset, y + 0.8f, mZ + zOffset, mXd, mYd, mZd,
                                        arrayWithLength<int>());
                }
            }
        }
    }

    if (Entity::instanceof(getTarget(), EntityType::PLAYER) && mLevel->getDifficulty() == Difficulty::PEACEFUL)
    {
        setTarget(boost::shared_ptr<LivingEntity>());
    }

    PIXEndNamedEvent();
}
float Wolf::getBodyRollAngle(float partialTick, float bodyRotation)
{
    float shake = (mShakeAnimO + (mShakeAnim - mShakeAnimO) * partialTick + bodyRotation) / 1.8f;
    if (shake < 0.0f)
    {
        shake = 0.0f;
    }
    else if (shake > 1.0f)
    {
        shake = 1.0f;
    }
    return Mth::sin(shake * WOLF_PI) * Mth::sin(shake * WOLF_PI * 11.0f) * 0.15f * WOLF_PI;
}
float Wolf::getHeadRollAngle(float partialTick)
{
    return ((mInterestedAngle - mInterestedAngleO) * partialTick + mInterestedAngleO) * 0.15f * WOLF_PI;
}
float Wolf::getEyeHeight()
{
    return static_cast<double>(mBbHeight * 0.8f);
}
int Wolf::getMaxHeadXRot()
{
    return isSitting() ? 20 : Mob::getMaxHeadXRot();
}
bool Wolf::hurt(const DamageSource* source, float damage)
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

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (mSitGoal != NULL)
        mSitGoal->wantToSit(false);
    if (attacker)
    {
        bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType());
        if (!isPlayer)
        {
            bool isArrow = eTYPE_DERIVED_FROM(EntityType::ARROW, attacker->GetType());
            if (!isArrow)
                damage = (damage + 1.0f) * 0.5f;
        }
    }
    return Animal::hurt(source, damage);
}
bool Wolf::doHurtTarget(boost::shared_ptr<Entity> target)
{
    EntityDamageSource* source = DamageSource::CreateMobAttack(boost::static_pointer_cast<Mob>(shared_from_this()));
    const bool hurtTarget = target->hurt(
        source, static_cast<float>(getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue()));
    delete source;
    if (hurtTarget)
        doEnchantDamageEffects(boost::static_pointer_cast<LivingEntity>(shared_from_this()), target);
    return hurtTarget;
}
void Wolf::setTame(bool tame)
{
    TamableAnimal::setTame(tame);
    if (tame)
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(20.0);
    else
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(4.0);
}
void Wolf::tame(boost::shared_ptr<Player> player, bool showParticles, bool)
{
    TamableAnimal::tame(player);
    mNavigation->stop();
    setTarget(boost::shared_ptr<LivingEntity>());
    setHealth(20.0f);
    spawnTamingParticles(showParticles);
}
bool Wolf::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);

    if (isTame())
    {
        if (!item->isEmpty())
        {
            FoodItem* food = dynamic_cast<FoodItem*>(item->getItem());
            if (food != NULL)
            {
                if (food->isMeat() && mEntityData->get(DATA_HEALTH_ID) < 20.0f)
                {
                    if (!player->mAbilities.mInstabuild)
                    {
                        item->shrink(1);
                    }
                    heal((float)food->getNutrition(item));
                    if (item->getCount() < 1)
                    {
                        player->mInventory->setItem(player->mInventory->m_selected, ItemInstance::EMPTY);
                    }
                    return true;
                }
            }
            else if (item->getItem() == Items::dye)
            {
                const DyeColor* color = DyeColor::byItemData(item->getAuxValue());
                if (color != getCollarColor())
                {
                    setCollarColor(color);
                    if (!player->mAbilities.mInstabuild)
                    {
                        item->shrink(1);
                    }
                    return true;
                }
            }
        }

        if (isOwnedBy(player))
        {
            if (!mLevel->mIsClientSide && !isFood(item) && !isLeashedToPlayer(player) && item->getItem() != Items::lead
                && (item->isEmpty() || item->getItem() != Items::spawnEgg || item->getAuxValue() != WOLF_SPAWN_EGG_AUX))
            {
                bool renaming = false;
                if (Item::getId(item->getItem()) == ItemId::NAME_TAG && !isPassenger() && item->hasCustomHoverName()
                    && (!hasCustomName() || item->getHoverName().compare(getCustomName()) != 0))
                {
                    renaming = true;
                }

                if (!renaming)
                {
                    mSitGoal->wantToSit(!isSitting());
                    mJumping = false;
                    mNavigation->stop();
                    setTarget(boost::shared_ptr<LivingEntity>());
                }
            }
        }

        return Animal::mobInteract(player, hand);
    }
    else
    {
        if (Item::getId(item->getItem()) == Item::getId(Items::bone) && !isAngry())
        {
            if (!player->mAbilities.mInstabuild)
            {
                item->shrink(1);
            }

            if (!mLevel->mIsClientSide)
            {
                if (mRandom->nextInt(3) == 0)
                {
                    player->awardStat(GenericStats::tamedEntity(EntityType::WOLF),
                                      GenericStats::param_tamedEntity(EntityType::WOLF));
                    tame(player, true, true);
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

        if (AgeUpBaby(player, item))
        {
            return true;
        }
        return AgableMob::mobInteract(player, hand);
    }
}
void Wolf::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::SHAKE_WETNESS)
    {
        mIsShaking = true;
        mShakeAnim = 0.0f;
        mShakeAnimO = 0.0f;
        return;
    }
    TamableAnimal::handleEntityEvent(event, data);
}
float Wolf::getTailAngle()
{
    if (isAngry())
    {
        return WOLF_PI * 0.49f;
    }

    if (isTame())
    {
        return (0.55f - (getMaxHealth() - mEntityData->get(DATA_HEALTH_ID)) * 0.02f) * WOLF_PI;
    }

    return WOLF_PI / 5.0f;
}
bool Wolf::isFood(const not_null_ptr<ItemInstance>& item)
{
    if (dynamic_cast<FoodItem*>(item->getItem()) == NULL)
    {
        return false;
    }

    return dynamic_cast<FoodItem*>(item->getItem())->isMeat();
}
int Wolf::GetSynchedHealth()
{
    return static_cast<int>(getEntityData()->get(DATA_HEALTH_ID));
}
unsigned int Wolf::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (isLeashedToPlayer(tooltipData.m_pPlayer))
        return IDS_TOOLTIPS_UNLEASH;

    bool baby;
    switch (Item::getId(tooltipData.m_pItemInstance->getItem()))
    {
    default:
        baby = isBaby();
        break;
    case ItemId::SPAWN_EGG:
    {
        if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == EntityType::WOLF)
            return IDS_TOOLTIPS_PLACE;
        if (isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
            return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
        return IDS_NONE;
    }
    case ItemId::NAME_TAG:
    {
        if (isPassenger())
            return IDS_NONE;
        if (tooltipData.m_pItemInstance->hasCustomHoverName())
        {
            if (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) != 0)
                return IDS_TOOLTIPS_NAME;
        }
        if (isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
            return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
        return IDS_NONE;
    }
    case ItemId::LEAD:
    {
        if (!isLeashed() && isOwnedBy(tooltipData.m_pPlayer))
            return IDS_TOOLTIPS_LEASH;
        return IDS_NONE;
    }
    case ItemId::BONE:
    {
        if (!isAngry() && !isTame())
            return getPassengers()->empty() ? IDS_TOOLTIPS_TAME : IDS_NONE;
        if (!isPassenger() && isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
            return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
        return IDS_NONE;
    }
    case ItemId::ENDER_PEARL:
    {
        if (isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
            return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
        return IDS_NONE;
    }
    case ItemId::DYE:
    {
        if (!isTame())
            return IDS_NONE;
        if (DyeColor::byItemData(tooltipData.m_pItemInstance->getAuxValue()) != getCollarColor())
            return IDS_TOOLTIPS_DYECOLLAR;
        if (isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
            return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
        return IDS_NONE;
    }
    }

    if (baby && isFood(tooltipData.m_pItemInstance))
        return IDS_TOOLTIPS_FEED;

    if (isTame())
    {
        if (!isFood(tooltipData.m_pItemInstance))
        {
            if (isOwnedBy(tooltipData.m_pPlayer) && !isInWater())
                return isSitting() ? IDS_TOOLTIPS_FOLLOWME : IDS_TOOLTIPS_SIT;
        }
        else
        {
            if (GetSynchedHealth() < 20)
                return IDS_TOOLTIPS_HEAL;
            if (!isInLove() && !isBaby() && !isBreedCoolingDown())
                return IDS_TOOLTIPS_LOVEMODE;
        }
    }
    return IDS_NONE;
}
boost::shared_ptr<AgableMob> Wolf::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<Wolf> offspring(new Wolf(mLevel));
    if (!getOwnerUUID().empty())
    {
        offspring->setOwnerUUID(getOwnerUUID());
        offspring->setTame(true);
        offspring->setHealth(offspring->getMaxHealth());
        offspring->setCollarColor(getCollarColor());
    }
    return offspring;
}
void Wolf::setIsInterested(bool interested)
{
    mEntityData->set(DATA_INTERESTED, interested, true);
}
bool Wolf::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other == shared_from_this())
        return false;
    if (!isTame())
        return false;
    bool isWolf = eTYPE_DERIVED_FROM(EntityType::WOLF, other->GetType());
    if (!isWolf)
        return false;
    boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(other);
    if (wolf == NULL)
        return false;
    if (!wolf->isTame())
        return false;
    if (wolf->isSitting())
        return false;
    return isInLove() && wolf->isInLove();
}
bool Wolf::wantsToAttack(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<LivingEntity> owner)
{
    if (eTYPE_DERIVED_FROM(EntityType::CREEPER, target->GetType())
        || eTYPE_DERIVED_FROM(EntityType::GHAST, target->GetType()))
    {
        return false;
    }

    if (eTYPE_DERIVED_FROM(EntityType::WOLF, target->GetType()))
    {
        boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(target);
        if (wolf->isTame() && wolf->getOwner() == owner)
        {
            return false;
        }
    }

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType())
        && eTYPE_DERIVED_FROM(EntityType::PLAYER, owner->GetType())
        && !boost::static_pointer_cast<Player>(owner)->canHarmPlayer(boost::static_pointer_cast<Player>(target)))
    {
        return false;
    }

    if (target->GetType() == EntityType::ABSTRACT_HORSE)
    {
        return !boost::static_pointer_cast<AbstractHorse>(target)->isTamed();
    }

    return true;
}
bool Wolf::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return !isAngry() && TamableAnimal::canBeLeashed(player);
}
unsigned long long Wolf::GetType()
{
    return EntityType::WOLF;
}
int Wolf::getMaxSpawnClusterSize()
{
    return 4;
}

Wolf::WolfAvoidEntityGoal::WolfAvoidEntityGoal(Wolf* wolf, const std::type_info& avoidType, float maxDistance,
                                               double walkSpeed, double sprintSpeed)
    : AvoidEntityGoal(wolf, avoidType, maxDistance, walkSpeed, sprintSpeed),
      mWolf(wolf)
{
}
bool Wolf::WolfAvoidEntityGoal::avoidLlama(const boost::shared_ptr<Llama>& llama)
{
    return llama->getStrength() >= mWolf->mRandom->nextInt(5);
}
bool Wolf::WolfAvoidEntityGoal::canUse()
{
    if (AvoidEntityGoal::canUse())
    {
        boost::shared_ptr<Entity> target = mToAvoid.lock();
        bool isLlama = eTYPE_DERIVED_FROM(EntityType::LLAMA, target->GetType());
        if (isLlama)
            return !mWolf->isTame() && avoidLlama(boost::static_pointer_cast<Llama>(target));
    }
    return false;
}
void Wolf::WolfAvoidEntityGoal::start()
{
    mWolf->setTarget(boost::shared_ptr<LivingEntity>());
    AvoidEntityGoal::start();
}
void Wolf::WolfAvoidEntityGoal::tick()
{
    mWolf->setTarget(boost::shared_ptr<LivingEntity>());
    AvoidEntityGoal::tick();
}
