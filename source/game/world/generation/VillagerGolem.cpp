#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/world/block/Predicates.hpp"

#include <typeinfo>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/world/generation/DefendVillageTargetGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/MoveTowardsTargetGoal.hpp"
#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MoveThroughVillageGoal.hpp"
#include "game/entity/ai/OfferFlowerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/item/ItemId.hpp"

const EntityDataAccessor<unsigned char>* VillagerGolem::DATA_PLAYER_CREATED_ID = NULL;

bool villagerGolemTargetPredicate(boost::shared_ptr<Entity> entity)
{
    return entity && eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType())
           && EntitySelector::VISIBLE_ENEMY_SELECTOR->apply(entity)
           && !eTYPE_DERIVED_FROM(EntityType::CREEPER, entity->GetType());
}

static Predicate<boost::shared_ptr<Entity> >* TARGET_SELECTOR =
    new Predicates<boost::shared_ptr<Entity> >::FunctionPredicate(villagerGolemTargetPredicate);

VillagerGolem::VillagerGolem(Level* level) : Golem(level)
{
    fjDerivedCtorCalls();
    mVillageCheckTime = 0;
    mVillage = boost::weak_ptr<Village>();
    mAttackAnimationTick = 0;
    mOfferFlowerTick = 0;
    setSize(1.4f, 2.7f);
}

Entity* VillagerGolem::create(Level* level)
{
    return new VillagerGolem(level);
}

void VillagerGolem::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::VILLAGER_GOLEM);
}

void VillagerGolem::staticCtor()
{
    DATA_PLAYER_CREATED_ID = SynchedEntityData::defineId<unsigned char>(EntityType::VILLAGER_GOLEM, EntityType::GOLEM,
                                                                        EntityDataSerializers::BYTE);
}

void VillagerGolem::registerGoals()
{
    mGoalSelector.addGoal(1, new MeleeAttackGoal(this, 1.0, true), true);
    mGoalSelector.addGoal(2, new MoveTowardsTargetGoal(this, 0.9, 32.0f), true);
    mGoalSelector.addGoal(3, new MoveThroughVillageGoal(this, 0.6, true), true);
    mGoalSelector.addGoal(4, new MoveTowardsRestrictionGoal(this, 1.0), true);
    mGoalSelector.addGoal(5, new OfferFlowerGoal(this), true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new DefendVillageTargetGoal(this), true);
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(
        3, new NearestAttackableTargetGoal(this, typeid(Mob), 0, false, true, TARGET_SELECTOR, false, false), true);
}

void VillagerGolem::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_PLAYER_CREATED_ID, static_cast<unsigned char>(0));
}

void VillagerGolem::customServerAiStep()
{
    if (--mVillageCheckTime < 1)
    {
        mVillageCheckTime = mRandom->nextInt(50) + 70;
        boost::shared_ptr<Village> village = mLevel->mVillages->getClosestVillage(
            BlockPos(Mth::floor(mX), Mth::floor(mY), Mth::floor(mZ)), 32);
        mVillage = village;
        Village* closest = village.get();
        if (closest == NULL)
            clearRestriction();
        else
        {
            BlockPos center = *closest->getCenter();
            restrictTo(center, static_cast<int>(closest->getRadius() * 0.6f));
        }
    }
    Mob::customServerAiStep();
}

void VillagerGolem::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(100.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->setBaseValue(1.0);
}

int VillagerGolem::decreaseAirSupply(int airSupply)
{
    return airSupply;
}

void VillagerGolem::doPush(boost::shared_ptr<Entity> entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::ENEMY, entity->GetType())
        && !eTYPE_DERIVED_FROM(EntityType::CREEPER, entity->GetType()) && getRandom()->nextInt(20) == 0)
        setTarget(boost::dynamic_pointer_cast<LivingEntity>(entity));
    LivingEntity::doPush(entity);
}

void VillagerGolem::aiStep()
{
    Mob::aiStep();
    if (mAttackAnimationTick > 0)
        --mAttackAnimationTick;
    if (mOfferFlowerTick > 0)
        --mOfferFlowerTick;
    if (mXd * mXd + mZd * mZd > MoveControl::MIN_SPEED_SQR && mRandom->nextInt(5) == 0)
    {
        int x = Mth::floor(mX);
        int y = Mth::floor(mY - 0.2f);
        int z = Mth::floor(mZ);
        const BlockState* state = mLevel->getBlockState(BlockPos(x, y, z));

        if (state->getMaterial() != Material::AIR)
        {
            arrayWithLength<int> params(1, true);
            params.data[0] = Block::getIdWithData(state);
            mLevel->addParticle(PARTICLE_BLOCK_CRACK, false, mX + (mRandom->nextFloat() - 0.5) * mBbWidth,
                                mBoundingBox->mMinY + 0.1, mZ + (mRandom->nextFloat() - 0.5) * mBbWidth,
                                4.0 * (mRandom->nextFloat() - 0.5), 0.5, (mRandom->nextFloat() - 0.5) * 4.0, params);
            delete params.data;
        }
    }
}

bool VillagerGolem::isPlayerCreated()
{
    return (mEntityData->get(DATA_PLAYER_CREATED_ID) & 1) != 0;
}

bool VillagerGolem::canAttackType(unsigned long long type)
{
    if (isPlayerCreated() && eTYPE_DERIVED_FROM(EntityType::PLAYER, type))
        return false;
    if (type == EntityType::CREEPER)
        return false;
    return Mob::canAttackType(type);
}

void VillagerGolem::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putBoolean(std::wstring(L"PlayerCreated"), isPlayerCreated());
}

void VillagerGolem::setPlayerCreated(bool playerCreated)
{
    unsigned char flags = mEntityData->get(DATA_PLAYER_CREATED_ID);
    if (playerCreated)
        mEntityData->set(DATA_PLAYER_CREATED_ID, static_cast<unsigned char>(flags | 1), true);
    else
        mEntityData->set(DATA_PLAYER_CREATED_ID, static_cast<unsigned char>(flags & ~1), true);
}

void VillagerGolem::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setPlayerCreated(tag->getBoolean(std::wstring(L"PlayerCreated")));
}

bool VillagerGolem::doHurtTarget(boost::shared_ptr<Entity> entity)
{
    mAttackAnimationTick = 10;
    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::START_ATTACKING, 0);
    EntityDamageSource* source = DamageSource::CreateMobAttack(boost::dynamic_pointer_cast<Mob>(shared_from_this()));
    bool hurt = entity->hurt(source, static_cast<float>(7 + mRandom->nextInt(15)));
    delete source;
    if (hurt)
    {
        entity->mYd += 0.4f;
        doEnchantDamageEffects(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()), entity);
    }
    playSound(SoundEvents::IRONGOLEM_ATTACK, 1.0f, 1.0f);
    return hurt;
}

void VillagerGolem::handleEntityEvent(unsigned char event, int data)
{
    switch (event)
    {
    case EntityEvent::START_ATTACKING:
        mAttackAnimationTick = 10;
        playSound(SoundEvents::IRONGOLEM_ATTACK, 1.0f, 1.0f);
        break;
    case EntityEvent::OFFER_FLOWER:
        mOfferFlowerTick = 400;
        break;
    case EntityEvent::STOP_OFFER_FLOWER:
        mOfferFlowerTick = 0;
        break;
    default:
        Mob::handleEntityEvent(event, data);
        break;
    }
}

unsigned int VillagerGolem::getAttackAnimationTick()
{
    return mAttackAnimationTick;
}

void VillagerGolem::offerFlower(bool offer)
{
    if (offer)
    {
        mOfferFlowerTick = 400;
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::OFFER_FLOWER, 0);
    }
    else
    {
        mOfferFlowerTick = 0;
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::STOP_OFFER_FLOWER, 0);
    }
}

const SoundEvent* VillagerGolem::getHurtSound(const DamageSource*)
{
    return SoundEvents::IRONGOLEM_HURT;
}

const SoundEvent* VillagerGolem::getDeathSound()
{
    return SoundEvents::IRONGOLEM_DEATH;
}

void VillagerGolem::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::IRONGOLEM_STEP, 1.0f, 1.0f);
}

ResourceLocation* VillagerGolem::getDefaultLootTable()
{
    return &BuiltInLootTables::IRON_GOLEM;
}

int VillagerGolem::getOfferFlowerTick()
{
    return mOfferFlowerTick;
}

boost::shared_ptr<Village> VillagerGolem::getVillage()
{
    return mVillage.lock();
}

void VillagerGolem::die(const DamageSource* source)
{
    if (!isPlayerCreated() && mLastHurtByPlayer && mVillage.lock())
        mVillage.lock()->modifyStanding(mLastHurtByPlayer->getName(), -5);
    LivingEntity::die(source);
}

bool VillagerGolem::hurt(const DamageSource* source, float damage)
{
    if (isPlayerCreated())
    {
        boost::shared_ptr<Entity> attacker = source->getDirectEntity();
        if (attacker && eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(attacker);
            if (!player->isAllowedToAttackPlayers())
                return false;
        }
    }
    clearRestriction();
    mVillageCheckTime = 5;
    return LivingEntity::hurt(source, damage);
}

unsigned int VillagerGolem::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::LEAD)
    {
        if (!isLeashed())
            return IDS_TOOLTIPS_LEASH;
        if (getLeashHolder().get() == static_cast<Entity*>(tooltipData.m_pPlayer.get()))
            return IDS_TOOLTIPS_UNLEASH;
    }
    return IDS_NONE;
}

unsigned int VillagerGolem::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!isPlayerCreated() || tooltipData.m_pPlayer->isAllowedToAttackPlayers())
        return IDS_TOOLTIPS_HIT;
    return IDS_NONE;
}

unsigned long long VillagerGolem::GetType()
{
    return EntityType::VILLAGER_GOLEM;
}
