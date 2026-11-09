#include "game/core/Llama.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/LlamaSpit.hpp"
#include "game/core/Mth.hpp"
#include <algorithm>
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/LlamaFollowCaravanGoal.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/RunAroundLikeCrazyGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"


static const int MAX_STRENGTH = 5;

const EntityDataAccessor<int>* Llama::DATA_STRENGTH = NULL;
const EntityDataAccessor<int>* Llama::DATA_SWAG = NULL;
const EntityDataAccessor<int>* Llama::DATA_VARIANT = NULL;

Llama::LlamaDefendTargetGoal::LlamaDefendTargetGoal(Llama* llama)
    : NearestAttackableTargetGoal(llama, typeid(Wolf), 16, false, true, NULL, false, false)
{
}
bool Llama::LlamaDefendTargetGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mTarget.lock();
    if (NearestAttackableTargetGoal::canUse() && target != NULL
        && eTYPE_DERIVED_FROM(EntityType::WOLF, target->GetType())
        && !boost::static_pointer_cast<Wolf>(target)->isTame())
        return true;
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    return false;
}
double Llama::LlamaDefendTargetGoal::getFollowDistance()
{
    return TargetGoal::getFollowDistance() * 0.25;
}

Llama::LlamaGroupData::LlamaGroupData(int variant) : MobGroupData(), mVariant(variant)
{
}
Llama::LlamaGroupData::~LlamaGroupData()
{
}

Llama::LlamaHurtByTargetGoal::LlamaHurtByTargetGoal(Llama* llama)
    : HurtByTargetGoal(llama, false, arrayWithLength<unsigned long long>())
{
}
bool Llama::LlamaHurtByTargetGoal::canContinueToUse()
{
    if (eTYPE_DERIVED_FROM(EntityType::LLAMA, mMob->GetType()))
    {
        Llama* llama = static_cast<Llama*>(mMob);
        if (llama->mDidSpit)
        {
            llama->setDidSpit(false);
            return false;
        }
    }

    return TargetGoal::canContinueToUse();
}

Llama::Llama(Level* level) : AbstractChestedHorse(level), mCaravanHead(), mCaravanTail()
{
    AbstractHorse::fjDerivedCtorCalls();
    mDidSpit = false;
    setSize(0.9f, 1.87f);
    internalSetSize(1.0f);
}
Llama::~Llama()
{
}
Entity* Llama::create(Level* level)
{
    return new Llama(level);
}
const DyeColor* Llama::getSwag()
{
    int blockData = mEntityData->get(DATA_SWAG);
    return blockData == -1 ? NULL : DyeColor::byBlockData(blockData);
}
void Llama::containerChanged(Container* container)
{
    const DyeColor* oldSwag = getSwag();
    AbstractHorse::containerChanged(container);
    const DyeColor* swag = getSwag();
    if (mTickCount > 20 && swag != NULL && swag != oldSwag)
        playSound(SoundEvents::LLAMA_SWAG, 0.5f, 1.0f);
}
void Llama::spit(const boost::shared_ptr<LivingEntity>& target)
{
    boost::shared_ptr<Llama> owner = boost::static_pointer_cast<Llama>(shared_from_this());
    boost::shared_ptr<LlamaSpit> spitEntity(new LlamaSpit(mLevel, owner));
    double dx = target->mX - mX;
    double dz = target->mZ - mZ;
    double dy = target->mBoundingBox->mMinY + target->mBbHeight / 3.0f - spitEntity->mY;
    float distance = Mth::sqrt(dx * dx + dz * dz) * 0.2f;
    spitEntity->shoot(dx, dy + distance, dz, 1.5f, 10.0f);
    float pitch = (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f;
    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::LLAMA_SPIT, getSoundSource(), 1.0f, pitch,
                      16.0f);
    mLevel->addEntity(spitEntity);
    mDidSpit = true;
}
void Llama::performRangedAttack(boost::shared_ptr<LivingEntity> target, float)
{
    spit(target);
}
bool Llama::isAggressive()
{
    return false;
}
void Llama::setAggressive(bool)
{
}
void Llama::staticCtor()
{
    DATA_STRENGTH = SynchedEntityData::defineId<int>(EntityType::LLAMA, EntityType::ABSTRACT_CHESTED_HORSE,
                                                     EntityDataSerializers::INT);
    DATA_SWAG = SynchedEntityData::defineId<int>(EntityType::LLAMA, EntityType::ABSTRACT_CHESTED_HORSE,
                                                 EntityDataSerializers::INT);
    DATA_VARIANT = SynchedEntityData::defineId<int>(EntityType::LLAMA, EntityType::ABSTRACT_CHESTED_HORSE,
                                                    EntityDataSerializers::INT);
}
void Llama::setStrength(int strength)
{
    mEntityData->set(DATA_STRENGTH, std::max(1, std::min(MAX_STRENGTH, strength)), true);
}
void Llama::setRandomStrength()
{
    int maxStrength = mRandom->nextFloat() < 0.04f ? 5 : 3;
    setStrength(mRandom->nextInt(maxStrength) + 1);
}
int Llama::getStrength()
{
    return mEntityData->get(DATA_STRENGTH);
}
int Llama::getVariant()
{
    return Mth::clamp(mEntityData->get(DATA_VARIANT), 0, 3);
}
void Llama::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractChestedHorse::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
    tag->putInt(L"Strength", getStrength());

    if (!mInventory->getItem(1)->isEmpty())
    {
        tag->put(L"DecorItem", mInventory->getItem(1)->save(new CompoundTag()));
    }
}
void Llama::setVariant(int variant)
{
    mEntityData->set(DATA_VARIANT, variant, true);
}
void Llama::readAdditionalSaveData(CompoundTag* tag)
{
    setStrength(tag->getInt(L"Strength"));
    AbstractChestedHorse::readAdditionalSaveData(tag);
    setVariant(tag->getInt(L"Variant"));
    if (tag->contains(L"DecorItem", Tag::TAG_COMPOUND))
        mInventory->setItem(1, not_null_ptr<ItemInstance>(new ItemInstance(tag->getCompound(L"DecorItem"))));
    updateEquipment();
}
void Llama::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new RunAroundLikeCrazyGoal(this, 1.2), true);
    mGoalSelector.addGoal(2, new LlamaFollowCaravanGoal(this, 2.1f), true);
    mGoalSelector.addGoal(3, new RangedAttackGoal(this, this, 1.25, 40, 20.0f), true);
    mGoalSelector.addGoal(3, new PanicGoal(this, 1.2), true);
    mGoalSelector.addGoal(4, new BreedGoal(this, 1.0), true);
    mGoalSelector.addGoal(5, new FollowParentGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 0.7), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new LlamaHurtByTargetGoal(this), true);
    mTargetSelector.addGoal(2, new LlamaDefendTargetGoal(this), true);
}
void Llama::registerAttributes()
{
    AbstractChestedHorse::registerAttributes();
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(40.0);
}
void Llama::defineSynchedData()
{
    AbstractChestedHorse::defineSynchedData();
    mEntityData->createDateItem(DATA_STRENGTH, 0);
    mEntityData->createDateItem(DATA_SWAG, -1);
    mEntityData->createDateItem(DATA_VARIANT, 0);
}
int Llama::getInventorySize()
{
    if (hasChest())
        return getInventoryColumns() * 3 + 2;
    return AbstractChestedHorse::getInventorySize();
}
void Llama::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!hasPassenger(passenger))
        return;
    if (RiderPositionOverrides::OverridePosition(shared_from_this(), passenger))
        return;

    const float cos = Mth::cos(mYBodyRot * Mth::DEG_TO_RAD);
    const float sin = Mth::sin(mYBodyRot * Mth::DEG_TO_RAD);
    passenger->setPos(mX + 0.3f * sin, mY + getRideHeight() + passenger->getRidingHeight(), mZ - 0.3f * cos);
}
double Llama::getRideHeight()
{
    return mBbHeight * 0.67;
}
bool Llama::canBeControlledByRider()
{
    return false;
}
int Llama::handleEating(const boost::shared_ptr<Player>& player, const not_null_ptr<ItemInstance>& item)
{
    float healAmount = 0.0f;
    int growth = 0;
    int temperGain = 0;
    int flag = 0;
    Item* food = item->getItem();

    if (food == Items::wheat)
    {
        healAmount = 2.0f;
        growth = 10;
        temperGain = 3;
    }
    else if (food == Item::byBlock(Blocks::hay_block))
    {
        healAmount = 10.0f;
        growth = 90;
        temperGain = 6;
        if (isTamed() && !isBaby())
            flag = CheckAndSetInLove(player);
    }

    if (getHealth() < getMaxHealth() && healAmount > 0.0f)
    {
        heal(healAmount);
        flag = 1;
    }

    if (isBaby() && growth > 0)
    {
        ageUp(growth, true);
        flag = 1;
    }

    if (temperGain > 0 && (flag || !isTamed()) && getTemper() < getMaxTemper())
    {
        flag = 1;
        modifyTemper(temperGain);
    }

    if (flag && !isSilent())
    {
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::EAT_ITEM_PARTICLES, Item::getId(food));
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::LLAMA_EAT, getSoundSource(), 1.0f,
                          (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f, 16.0f);
    }

    return flag;
}
bool Llama::isImmobile()
{
    return !(getHealth() > 0.0f) || isEating();
}
MobGroupData* Llama::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                      const std::wstring& name)
{
    MobGroupData* result = AbstractHorse::finalizeMobSpawn(difficulty, data, name);
    setRandomStrength();
    if (dynamic_cast<LlamaGroupData*>(result) != NULL)
    {
        setVariant(static_cast<LlamaGroupData*>(result)->mVariant);
    }
    else
    {
        int variant = mRandom->nextInt(4);
        result = new LlamaGroupData(variant);
        setVariant(variant);
    }
    return result;
}
bool Llama::hasSwag()
{
    return getSwag() != NULL;
}
const SoundEvent* Llama::getAngrySound()
{
    return SoundEvents::LLAMA_ANGRY;
}
const SoundEvent* Llama::getAmbientSound()
{
    return SoundEvents::LLAMA_AMBIENT;
}
const SoundEvent* Llama::getHurtSound(const DamageSource*)
{
    return SoundEvents::LLAMA_HURT;
}
const SoundEvent* Llama::getDeathSound()
{
    return SoundEvents::LLAMA_DEATH;
}
void Llama::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::LLAMA_STEP, 0.15f, 1.0f);
}
void Llama::playChestEquipsSound()
{
    playSound(SoundEvents::LLAMA_CHEST, 1.0f, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
}
void Llama::makeMad()
{
    const SoundEvent* sound = getAngrySound();
    if (sound != NULL)
    {
        playSound(sound, getSoundVolume(), getVoicePitch());
    }
}
ResourceLocation* Llama::getDefaultLootTable()
{
    return &BuiltInLootTables::LLAMA;
}
int Llama::getInventoryColumns()
{
    return getStrength();
}
bool Llama::wearsArmor()
{
    return true;
}
bool Llama::isArmor(const not_null_ptr<ItemInstance>& item)
{
    return item->getItem() == Item::byBlock(Blocks::carpet);
}
bool Llama::canBeSaddled() const
{
    return false;
}
unsigned int Llama::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
        return IDS_NONE;

    unsigned int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
    bool food = itemId == BlockId::HAY_BLOCK || itemId == ItemId::WHEAT;
    bool hay = itemId == BlockId::HAY_BLOCK;

    if (itemId == ItemId::SPAWN_EGG)
    {
        if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == EntityType::LLAMA)
            return IDS_TOOLTIPS_PLACE;
        if (isLeashedToPlayer(tooltipData.m_pPlayer))
            return IDS_TOOLTIPS_UNLEASH;
        return IDS_NONE;
    }

    if (isLeashedToPlayer(tooltipData.m_pPlayer))
        return IDS_TOOLTIPS_UNLEASH;

    if (itemId == ItemId::LEAD)
    {
        if (!isLeashed())
            return IDS_TOOLTIPS_LEASH;
        if (getPassengers()->empty() && isTamed())
            return IDS_TOOLTIPS_MOUNT;
        return IDS_NONE;
    }

    if (itemId == ItemId::NAME_TAG && (!tooltipData.m_pPlayer->isSneaking() || !isTamed()))
    {
        if (tooltipData.m_pItemInstance->hasCustomHoverName()
            && (!hasCustomName() || !(tooltipData.m_pItemInstance->getHoverName() == getCustomName())))
            return IDS_TOOLTIPS_NAME;
        if (!isTamed() || !getPassengers()->empty())
            return IDS_NONE;
        return IDS_TOOLTIPS_MOUNT;
    }

    bool baby = isBaby();
    if (baby || !isTamed())
    {
        if (!baby && tooltipData.m_pItemInstance->isEmpty())
        {
            if (!getPassengers()->empty())
                return IDS_NONE;
            return IDS_TOOLTIPS_TAME;
        }
        if (food)
            return IDS_TOOLTIPS_FEED;
        return IDS_NONE;
    }

    if ((tooltipData.m_pPlayer->isSneaking() && (!isVehicle() || hasPassenger(tooltipData.m_pPlayer)) && isTamed())
        || (itemId == ItemId::SADDLE && !isSaddled()))
        return IDS_TOOLTIPS_OPEN;
    if (itemId == BlockId::CHEST && !hasChest())
        return IDS_TOOLTIPS_ATTACH;
    if (hay && !isInLove() && !isBaby() && getPassengers()->empty() && !isBreedCoolingDown())
        return IDS_TOOLTIPS_LOVEMODE;
    if (food && getHealth() < getMaxHealth())
        return IDS_TOOLTIPS_HEAL;
    if (getPassengers()->empty())
        return IDS_TOOLTIPS_MOUNT;
    return IDS_NONE;
}
bool Llama::HideUseTooltips()
{
    return true;
}
void Llama::setSwag(const DyeColor* color)
{
    mEntityData->set(DATA_SWAG, color != NULL ? color->getBlockData() : -1, true);
}

void Llama::setSwagByItem(const not_null_ptr<ItemInstance>& item)
{
    if (isArmor(item))
        setSwag(DyeColor::byBlockData(item->getAuxValue()));
    else
        setSwag(NULL);
}
void Llama::updateEquipment()
{
    if (mLevel->mIsClientSide)
        return;

    AbstractHorse::updateEquipment();
    not_null_ptr<ItemInstance> item = mInventory->getItem(1);
    setSwagByItem(item);
}
int Llama::getMaxTemper() const
{
    return 30;
}
bool Llama::canMate(const boost::shared_ptr<Animal>& other)
{
    return other != shared_from_this() && eTYPE_DERIVED_FROM(EntityType::LLAMA, other->GetType()) && canParent()
           && boost::static_pointer_cast<Llama>(other)->canParent();
}
boost::shared_ptr<AgableMob> Llama::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<Llama> baby(new Llama(mLevel));
    AbstractHorse::setOffspringAttributes(partner, baby);

    boost::shared_ptr<Llama> llamaPartner = boost::static_pointer_cast<Llama>(partner);
    int strength = mRandom->nextInt(std::max(getStrength(), llamaPartner->getStrength())) + 1;
    if (mRandom->nextFloat() < 0.03f)
        strength++;
    baby->setStrength(strength);

    baby->setVariant(mRandom->nextBoolean() ? getVariant() : llamaPartner->getVariant());

    return baby;
}
void Llama::setDidSpit(bool didSpit)
{
    mDidSpit = didSpit;
}
void Llama::causeFallDamage(float fallDistance, float damageMultiplier)
{
    float damage = Mth::ceil((fallDistance * 0.5f - 3.0f) * damageMultiplier);
    if (damage > 0.0f)
    {
        if (fallDistance >= 6.0f)
        {
            hurt(DamageSource::FALL, damage);
            if (isVehicle())
            {
                std::set<boost::shared_ptr<Entity> >* passengers = getIndirectPassengers();
                for (std::set<boost::shared_ptr<Entity> >::iterator it = passengers->begin(); it != passengers->end();
                     ++it)
                {
                    (*it)->hurt(DamageSource::FALL, damage);
                }
                delete passengers;
            }
        }

        const BlockState* state = mLevel->getBlockState(BlockPos(mX, mY - 0.2 - static_cast<double>(mYRotO), mZ));
        if (state->getMaterial() != Material::AIR && !isSilent())
        {
            const SoundType* soundType = state->getBlock()->getSoundType();
            mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, soundType->getStepSound(), getSoundSource(),
                              soundType->getVolume() * 0.5f, soundType->getPitch() * 0.75f, 16.0f);
        }
    }
}
void Llama::leaveCaravan()
{
    boost::shared_ptr<Llama> head = mCaravanHead.lock();
    if (head)
        head->mCaravanTail.reset();
    mCaravanHead.reset();
}

void Llama::joinCaravan(const boost::shared_ptr<Llama>& llama)
{
    mCaravanHead = llama;
    llama->mCaravanTail = boost::static_pointer_cast<Llama>(shared_from_this());
}
bool Llama::hasCaravanTail()
{
    return mCaravanTail.lock().get() != NULL;
}

bool Llama::inCaravan()
{
    return mCaravanHead.lock().get() != NULL;
}
boost::shared_ptr<Llama> Llama::getCaravanHead()
{
    return mCaravanHead.lock();
}
double Llama::followLeashSpeed()
{
    return 2.0;
}
void Llama::followMommy()
{
    if (!inCaravan() && isBaby())
    {
        AbstractHorse::followMommy();
    }
}
bool Llama::canEatGrass() const
{
    return false;
}
unsigned long long Llama::GetType()
{
    return EntityType::LLAMA;
}
