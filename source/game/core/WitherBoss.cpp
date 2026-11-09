#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <algorithm>
#include <typeinfo>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mth.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/WitherTargetHighestDamage.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LivingEntitySelector.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/ai/WitherRandomAttackPosGoal.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/server/ServerBossEvent.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"


const EntityDataAccessor<int>* WitherBoss::DATA_TARGET_A = NULL;
const EntityDataAccessor<int>* WitherBoss::DATA_TARGET_B = NULL;
const EntityDataAccessor<int>* WitherBoss::DATA_TARGET_C = NULL;
const EntityDataAccessor<int>* WitherBoss::DATA_TARGETS[3] = {NULL, NULL, NULL};
const EntityDataAccessor<int>* WitherBoss::DATA_ID_INV = NULL;
const EntityDataAccessor<bool>* WitherBoss::DATA_AERIAL_ATTACK = NULL;
const EntityDataAccessor<bool>* WitherBoss::DATA_CHARGE_ATTACK = NULL;
Predicate<boost::shared_ptr<Entity> >* WitherBoss::LIVING_ENTITY_SELECTOR = new LivingEntitySelector();

WitherBoss::WitherBoss(Level* level)
    : Monster(level),
      mInitialShieldHealth(40),
      mPhase(1),
      mShotDelay(0),
      mPauseTicks(0),
      mDyingFrames(0),
      mDyingFlashInterval(15),
      mOverlayAlpha(1.0f),
      mSwellAmount(0.0f),
      mOldSwellAmount(0.0f),
      mChargeDelay(0),
      mCharging(false),
      mHeadShotCooldown(3),
      mFireRate(20),
      mShotTicks(20),
      mTargetRange(30.0f),
      mSpawningFrames(200),
      mDiving(false),
      mChargeTimer(0),
      mChargeDirection(NULL),
      mWantsToMove(false),
      mChargeTicks(0),
      mTicksSinceShot(0),
      mShieldHealth(40),
      mShotCount(0),
      mChargeInterval(40),
      mSkipNextCharge(false),
      mLastHealthInterval(-1),
      mSpinSpeed(10.0f),
      mDestroyBlocksTick(5),
      mIsPathing(false)
{
    defineSynchedData();
    registerAttributes();
    fjDerivedCtorCalls();

    mBossEvent = static_cast<ServerBossEvent*>(
        (new ServerBossEvent(IDS_WITHER, L"", BossEvent::PURPLE, BossEvent::PROGRESS))->setDarkenScreen(true));

    for (int head = 0; head < 2; ++head)
    {
        mXRotHeads[head] = 0.0f;
        mYRotHeads[head] = 0.0f;
        mXRotOHeads[head] = 0.0f;
        mYRotOHeads[head] = 0.0f;
        mNextHeadUpdate[head] = 0;
        mIdleHeadUpdates[head] = 0;
    }

    mFireImmune = true;
    static_cast<GroundPathNavigation*>(getNavigation())->setCanFloat(true);
    mExperienceReward = 50;
    mCategories |= EntityCategory::eEntityCategory_Undead | EntityCategory::eEntityCategory_Monster;
    setSize(1.0f, 3.5f);
    setAerialAttack(true);

    mMaxHealth = (int)getMaxHealth();
    setHealth(1.0f);
    mBossEvent->setPercent(0.0f);
    mLastHealthInterval = (int)getMaxHealth();
    mPhaseHealth = (int)getMaxHealth() / 2;
    mHealthInterval = (int)getMaxHealth() / 6;

    mDeathSource = DamageSource::OUT_OF_WORLD->copy();
    mFlyingSpeed = 0.05f;
}

WitherBoss::~WitherBoss()
{
    delete mDeathSource;
}

Entity* WitherBoss::create(Level* level)
{
    return new WitherBoss(level);
}

void WitherBoss::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::WITHER_BOSS);
}

void WitherBoss::staticCtor()
{
    DATA_TARGET_A = SynchedEntityData::defineId<int>(EntityType::WITHER_BOSS, EntityType::MONSTER,
                                                     EntityDataSerializers::INT);
    DATA_TARGET_B = SynchedEntityData::defineId<int>(EntityType::WITHER_BOSS, EntityType::MONSTER,
                                                     EntityDataSerializers::INT);
    DATA_TARGET_C = SynchedEntityData::defineId<int>(EntityType::WITHER_BOSS, EntityType::MONSTER,
                                                     EntityDataSerializers::INT);

    const EntityDataAccessor<int>* targets[3] = {DATA_TARGET_A, DATA_TARGET_B, DATA_TARGET_C};
    memcpy(DATA_TARGETS, targets, sizeof(DATA_TARGETS));

    DATA_ID_INV = SynchedEntityData::defineId<int>(EntityType::WITHER_BOSS, EntityType::MONSTER,
                                                   EntityDataSerializers::INT);
    DATA_AERIAL_ATTACK = SynchedEntityData::defineId<bool>(EntityType::WITHER_BOSS, EntityType::MONSTER,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_CHARGE_ATTACK = SynchedEntityData::defineId<bool>(EntityType::WITHER_BOSS, EntityType::MONSTER,
                                                           EntityDataSerializers::BOOLEAN);
}

bool WitherBoss::canDestroy(Block* block)
{
    return block != Blocks::bedrock && block != Blocks::end_portal && block != Blocks::end_portal_frame
           && block != Blocks::command_block && block != Blocks::barrier && block != Blocks::structure_block
           && block != Blocks::structure_void && block != Blocks::piston_extension && block != Blocks::end_gateway;
}

unsigned long long WitherBoss::GetType()
{
    return EntityType::WITHER_BOSS;
}

void WitherBoss::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->define(DATA_TARGET_A, getId());
    mEntityData->define(DATA_TARGET_B, getId());
    mEntityData->define(DATA_TARGET_C, getId());
    mEntityData->define(DATA_ID_INV, 0);
    mEntityData->define(DATA_AERIAL_ATTACK, false);
    mEntityData->define(DATA_CHARGE_ATTACK, false);
}

void WitherBoss::registerAttributes()
{
    Monster::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::FLYING_SPEED);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(600.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.6f);

    if (mLevel->getDifficulty() == Difficulty::EASY)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(getMaxHealth() * 0.5f);
        mInitialShieldHealth /= 2;
    }
    else if (mLevel->getDifficulty() == Difficulty::NORMAL)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(getMaxHealth() * 0.75f);
        mInitialShieldHealth = (int)(mInitialShieldHealth * 0.75f);
    }

    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(70.0);
    getAttribute(SharedMonsterAttributes::ARMOR)->setBaseValue(4.0);
}

void WitherBoss::causeFallDamage(float fallDistance, float damageModifier)
{
}

int WitherBoss::getLightColor()
{
    return 0xF000F0;
}

bool WitherBoss::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    boost::shared_ptr<Entity> directEntity = source->getDirectEntity();
    if (directEntity != NULL && eTYPE_DERIVED_FROM(EntityType::WITHER_SKULL, directEntity->GetType()))
        mPauseTicks = 80;

    if (source == DamageSource::DROWN
        || (source->getEntity() != NULL && eTYPE_DERIVED_FROM(EntityType::WITHER_BOSS, source->getEntity()->GetType())))
        return false;

    if (mSpawningFrames > 0 && source->getEntity() != NULL)
    {
        if (eTYPE_DERIVED_FROM(EntityType::MOB, source->getEntity()->GetType()))
        {
            boost::shared_ptr<Mob> attacker = boost::static_pointer_cast<Mob>(
                mLevel->getEntity(source->getEntity()->getId()));
            attacker->knockback(shared_from_this(), damage, mX - attacker->mX, mY - attacker->mY);
        }
    }

    if ((getInvulnerableTicks() > 0 && source != DamageSource::OUT_OF_WORLD) || mDyingFrames > 0)
        return false;

    if (isPowered() && directEntity != NULL)
    {
        if (eTYPE_DERIVED_FROM(EntityType::ARROW, directEntity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::THROWN_TRIDENT, directEntity->GetType()))
            return false;
    }

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker != NULL && !eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType()))
    {
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, attacker->GetType())
            && boost::static_pointer_cast<LivingEntity>(attacker)->getMobType() == MOB_TYPE_UNDEAD)
            return false;
    }

    if (mDestroyBlocksTick < 1)
        mDestroyBlocksTick = 20;
    for (int head = 0; head < 2; ++head)
        mIdleHeadUpdates[head] += 3;

    bool hurt = Monster::hurt(source, damage);
    if (hurt)
    {
        int health = (int)getHealth();
        if (mLevel->getDifficulty() != Difficulty::EASY && health < mLastHealthInterval - mHealthInterval)
        {
            mFireRate /= 2;
            mLastHealthInterval -= mHealthInterval;
        }

        if (health <= mPhaseHealth && mPhase > 0)
            changePhase();

        if (attacker != NULL)
        {
            bool canSeeAttacker = canSee(attacker);
            float distanceSqr = getPos(1.0f)->subtract(attacker->mX, attacker->mY, attacker->mZ)->lengthSqr();
            if (attacker == getTarget() && (!canSeeAttacker || mTargetRange * mTargetRange < distanceSqr))
                mChargeTimer = 1;
        }

        if (mBossEvent != NULL && attacker != NULL
            && eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, attacker->GetType()))
            mBossEvent->addPlayer(boost::static_pointer_cast<ServerPlayer>(attacker), (int)damage);
    }
    return hurt;
}

bool WitherBoss::isPickable()
{
    return !mRemoved;
}

void WitherBoss::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setInvulnerableTicks(tag->getInt(L"Invul"));

    if (hasCustomName())
        mBossEvent->setName(getDisplayName());

    bool legacySave = true;
    if (tag->contains(L"Phase"))
    {
        mPhase = tag->getInt(L"Phase");
        legacySave = false;
    }
    if (tag->contains(L"AirAttack"))
        setAerialAttack(tag->getBoolean(L"AirAttack"));
    if (tag->contains(L"ShieldHealth"))
        mShieldHealth = tag->getInt(L"ShieldHealth");
    if (tag->contains(L"maxHealth"))
        mMaxHealth = tag->getInt(L"maxHealth");
    if (tag->contains(L"lastHealthInterval"))
        mLastHealthInterval = tag->getInt(L"lastHealthInterval");
    if (tag->contains(L"dyingFrames"))
        mDyingFrames = tag->getInt(L"dyingFrames");
    if (tag->contains(L"swellAmount"))
        mSwellAmount = tag->getFloat(L"swellAmount");
    if (tag->contains(L"oldSwellAmount"))
        mOldSwellAmount = tag->getFloat(L"oldSwellAmount");
    if (tag->contains(L"firerate"))
        mFireRate = tag->getInt(L"firerate");

    bool notInvulnerable = getInvulnerableTicks() <= 0;
    if (tag->contains(L"SpawningFrames"))
        mSpawningFrames = tag->getInt(L"SpawningFrames");
    else if (notInvulnerable)
        mSpawningFrames = 0;

    if (tag->contains(L"overlayAlpha"))
        mOverlayAlpha = tag->getFloat(L"overlayAlpha");
    else if (mSpawningFrames > 0)
        mOverlayAlpha = 1.0f;
    else
        mOverlayAlpha = 0.0f;

    if (legacySave)
    {
        int maxHealth = (int)getMaxHealth();
        int health = (int)getHealth();
        mLastHealthInterval = health;
        mPhaseHealth = maxHealth / 2;
        mHealthInterval = maxHealth / 6;
        if (health <= mPhaseHealth)
            mPhase = 0;
    }
}

void WitherBoss::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putBoolean(L"AirAttack", getAerialAttack());
    tag->putInt(L"ShieldHealth", mShieldHealth);
    tag->putInt(L"Phase", mPhase);
    tag->putInt(L"SpawningFrames", mSpawningFrames);
    tag->putInt(L"maxHealth", mMaxHealth);
    tag->putInt(L"lastHealthInterval", mLastHealthInterval);
    tag->putInt(L"dyingFrames", mDyingFrames);
    tag->putFloat(L"overlayAlpha", mOverlayAlpha);
    tag->putFloat(L"swellAmount", mSwellAmount);
    tag->putFloat(L"oldSwellAmount", mOldSwellAmount);
    tag->putInt(L"firerate", mFireRate);
}

void WitherBoss::readAdditionContructionTag(CompoundTag* tag)
{
    setAerialAttack(tag->getBoolean(L"AirAttack"));
    mShieldHealth = tag->getInt(L"ShieldHealth");
    mPhase = tag->getInt(L"Phase");
    mSpawningFrames = tag->getInt(L"SpawningFrames");
    mMaxHealth = tag->getInt(L"maxHealth");
    mLastHealthInterval = tag->getInt(L"lastHealthInterval");
    mDyingFrames = tag->getInt(L"dyingFrames");
    mOverlayAlpha = tag->getFloat(L"overlayAlpha");
    mSwellAmount = tag->getFloat(L"swellAmount");
    mOldSwellAmount = tag->getFloat(L"oldSwellAmount");
    mFireRate = tag->getInt(L"firerate");
}

CompoundTag* WitherBoss::getAdditionContructionTag()
{
    CompoundTag* tag = Entity::getAdditionContructionTag();
    if (tag == NULL)
        tag = new CompoundTag();

    tag->putBoolean(L"AirAttack", getAerialAttack());
    tag->putInt(L"ShieldHealth", mShieldHealth);
    tag->putInt(L"Phase", mPhase);
    tag->putInt(L"SpawningFrames", mSpawningFrames);
    tag->putInt(L"maxHealth", mMaxHealth);
    tag->putInt(L"lastHealthInterval", mLastHealthInterval);
    tag->putInt(L"dyingFrames", mDyingFrames);
    tag->putFloat(L"overlayAlpha", mOverlayAlpha);
    tag->putFloat(L"swellAmount", mSwellAmount);
    tag->putFloat(L"oldSwellAmount", mOldSwellAmount);
    tag->putInt(L"firerate", mFireRate);
    return tag;
}

bool WitherBoss::canRide(boost::shared_ptr<Entity> entity)
{
    return false;
}

void WitherBoss::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::DEATH)
    {
        mDyingFrames = 200;
        return;
    }
    Mob::handleEntityEvent(event, data);
}

bool WitherBoss::canFly()
{
    return true;
}

void WitherBoss::makeStuckInWeb()
{
}

bool WitherBoss::canChangeDimensions()
{
    return false;
}

std::wstring WitherBoss::getDisplayName()
{
    if (hasCustomName())
        return getCustomName();
    return L10N::GetString(IDS_WITHER);
}

void WitherBoss::setCustomName(const std::wstring& name, const std::wstring& displayName, bool visible)
{
    Entity::setCustomName(name, displayName, visible);
    mBossEvent->setName(getDisplayName());
}

void WitherBoss::startSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    Entity::startSeenByPlayer(player);
    mBossEvent->addPlayer(player, 0);
}

void WitherBoss::stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
    Entity::stopSeenByPlayer(player);
    mBossEvent->removePlayer(player);
}

void WitherBoss::addEffect(MobEffectInstance* effectInstance, const boost::shared_ptr<Entity>& source)
{
}

void WitherBoss::heal(float amount)
{
    if (getHealth() > 0.0f && !mLevel->mIsClientSide)
        LivingEntity::heal(amount);
}

float WitherBoss::getHealth()
{
    return LivingEntity::getHealth();
}

void WitherBoss::die(const DamageSource* source)
{
    mDyingFrames = 200;
    mDeathSource = source->copy();
    mCharging = false;
    mPauseTicks = 0;
    mChargeDelay = 0;
    playSound(getDeathSound(), 0.5f, 1.0f);
    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::DEATH, 0);
}

const SoundEvent* WitherBoss::getHurtSound(const DamageSource* source)
{
    return SoundEvents::WITHER_HURT;
}

const SoundEvent* WitherBoss::getDeathSound()
{
    return SoundEvents::WITHER_DEATH;
}

void WitherBoss::dropDeathLoot(bool hitByPlayer, int lootingLevel)
{
    boost::shared_ptr<ItemEntity> netherStar = spawnAtLocation(Items::netherStar, 1);
    if (netherStar != NULL)
        netherStar->setExtendedLifetime();

    if (!mLevel->mIsClientSide)
    {
        std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(
            typeid(Player), mBoundingBox->grow(50.0, 100.0, 50.0));
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
        {
            boost::static_pointer_cast<Player>(*it)->awardStat(GenericStats::killWither(),
                                                               GenericStats::param_killWither());
        }
        delete players;
    }
}

int WitherBoss::getArmorValue()
{
    return 4;
}

EMobType WitherBoss::getMobType()
{
    return MOB_TYPE_UNDEAD;
}

void WitherBoss::aiStep()
{
    generateParticles();

    int pauseTicks = mPauseTicks;
    if (mShotDelay > 0)
        --mShotDelay;
    mPauseTicks = pauseTicks - 1;
    if (mPauseTicks > 0)
        return;

    mYd *= 0.6f;

    if (mDyingFrames > 0)
    {
        mDeathTime = -1;
        if (mDyingFrames == 200)
            mOverlayAlpha = 0.0f;

        --mDyingFrames;
        if (mDyingFrames == 0)
        {
            if (!mLevel->mIsClientSide)
            {
                mLevel->explode(shared_from_this(), mX, mY + getEyeHeight(), mZ, 7.0f, false,
                                mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING), false, false, false,
                                -1.0f);
                LivingEntity::die(mDeathSource);

                if (!mLevel->mIsClientSide && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT))
                {
                    int experience = getExperienceReward(mLastHurtByPlayer);
                    while (experience > 0)
                    {
                        int value = ExperienceOrb::getExperienceValue(experience);
                        experience -= value;
                        mLevel->addEntity(boost::shared_ptr<Entity>(new ExperienceOrb(mLevel, mX, mY, mZ, value)));
                    }
                }

                remove();
                return;
            }
        }
        else if (mDyingFrames % mDyingFlashInterval != 0)
        {
            goto dyingSounds;
        }

        setAerialAttack(!getAerialAttack());
        if (mDyingFlashInterval > 1)
            --mDyingFlashInterval;

    dyingSounds:
        if (mDyingFrames == 120)
            playSound(SoundEvents::WITHER_DEATH, 1.0f, 1.0f);
        else if (mDyingFrames == 60)
            playSound(SoundEvents::WITHER_DEATH, 0.75f, 1.0f);

        setInvulnerableTicks(mDyingFrames);
        mOverlayAlpha += 0.005f;
        mOldSwellAmount = mSwellAmount;
        mSwellAmount = mSwellAmount + 1.0f;
        return;
    }

    int health = (int)getHealth();
    if (mSpawningFrames > 0 && health > 0)
    {
        --mSpawningFrames;
        if (mSpawningFrames == 0)
        {
            if (!mLevel->mIsClientSide)
            {
                setAerialAttack(true);
                mShieldHealth = mInitialShieldHealth;
                mLevel->explode(shared_from_this(), mX, mY + getEyeHeight(), mZ, 7.0f, false,
                                mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING), false, false, false,
                                -1.0f);
            }
            playSoundToAllPlayers(SoundEvents::WITHER_SPAWN, 1.0f, 1.0f);
        }

        setInvulnerableTicks(mSpawningFrames);
        heal(ceilf(getMaxHealth() / 200.0f));

        float oldBodyRot = mYBodyRot;
        mSpinSpeed += 0.2f;
        float bodyRot = Mth::wrapDegrees(oldBodyRot - mSpinSpeed);
        mYBodyRot = bodyRot;
        mYHeadRot = bodyRot;
        mYRot = bodyRot;
        if ((oldBodyRot < 0.0f && bodyRot >= 0.0f) || mSpawningFrames == 199)
            playSound(SoundEvents::WITHER_SPAWN, 1.0f, 1.0f);

        mYRotHeads[0] = Mth::wrapDegrees(mYRotHeads[0] + mSpinSpeed);
        mYRotHeads[1] = Mth::wrapDegrees(mYRotHeads[1] + mSpinSpeed);
        Monster::aiStep();
        return;
    }

    if (!mLevel->mIsClientSide && getAlternativeTarget(0) != getId())
    {
        boost::shared_ptr<Entity> target = mLevel->getEntity(getAlternativeTarget(0));
        if (target != NULL)
        {
            if (!mIsPathing)
            {
                if ((target->mY <= mY || mDiving) && (isPowered() || mY >= target->mY + 5.0))
                {
                    if (mDiving)
                        mYd -= 1.0;
                }
                else
                {
                    if (mYd < 0.0)
                        mYd = 0.0;
                    mYd += (0.5 - mYd) * 0.6;
                }
            }
            else if (getMoveControl() != NULL)
            {
                mYd = getMoveControl()->getWantedY() - mY;
            }
        }
    }

    float bodyXRot = 90.0f;
    if (mChargeTicks > 0 && mChargeDirection != NULL)
    {
        double chargeX = mChargeDirection->x;
        double chargeZ = mChargeDirection->z;
        --mChargeTicks;
        if (mChargeTicks == 0)
        {
            mChargeDirection->x = 0.0;
            mXd = 0.0;
            mZd = 0.0;
            mShotDelay = 20;
            mCharging = false;
        }
        else
        {
            mZd = chargeZ + chargeZ;
            mXd = chargeX + chargeX;
        }
    }

    boost::shared_ptr<LivingEntity> target = getTarget();
    if (target != NULL)
    {
        Vec3* myPos = getAttachPos(ENTITY_LOCATION_HEAD, 0.0f);
        Vec3* direction = target->getAttachPos(ENTITY_LOCATION_HEAD, 0.0f)->subtract(myPos);
        int directionX = (int)direction->x;
        float yaw = (float)(Mth::atan2((int)direction->z, directionX) * 57.295776f - bodyXRot);
        mYHeadRot = Mth::clampRotateDegrees(mYHeadRot, yaw, 10.0f);
        if (IsChargeAttacking())
            mYBodyRot = mYHeadRot;
    }
    Monster::aiStep();

    if (!mLevel->mIsClientSide)
    {
        if (IsChargeAttacking())
            mXRot = bodyXRot;
        else
            mXRot = 0.0f;
    }

    mXRotOHeads[0] = mXRotHeads[0];
    mXRotOHeads[1] = mXRotHeads[1];
    mYRotOHeads[0] = mYRotHeads[0];
    mYRotOHeads[1] = mYRotHeads[1];

    for (int head = 0; head < 2; ++head)
    {
        int targetId = getAlternativeTarget(head + 1);
        boost::shared_ptr<Entity> headTarget;
        if (targetId != getId())
            headTarget = mLevel->getEntity(targetId);

        if (headTarget != NULL)
        {
            double headX = getHeadX(head + 1);
            double headY = getHeadY(head + 1);
            double headZ = getHeadZ(head + 1);
            double xd = headTarget->mX - headX;
            double yd = headTarget->mY + headTarget->getEyeHeight() - headY;
            double zd = headTarget->mZ - headZ;
            double horizontalDistance = Mth::sqrt(xd * xd + zd * zd);
            double yaw = atan2(zd, xd);
            double pitch = atan2(yd, horizontalDistance);
            mXRotHeads[head] = rotlerp(mXRotHeads[head], -(float)(pitch * 57.295776f), 40.0f);
            mYRotHeads[head] = rotlerp(mYRotHeads[head], (float)(yaw * 57.295776f) - bodyXRot, 10.0f);
        }
        else
        {
            mYRotHeads[head] = rotlerp(mYRotHeads[head], mYBodyRot, 10.0f);
            mXRotHeads[head] = rotlerp(mXRotHeads[head], mXRot, 40.0f);
        }
    }

    if (canShoot())
    {
        boost::shared_ptr<LivingEntity> shootTarget = getTarget();
        if (shootTarget != NULL && ++mTicksSinceShot > 100)
        {
            mChargeTimer = 1;
            mTicksSinceShot = 0;
        }
    }
}

void WitherBoss::registerGoals()
{
    mGoalSelector.addGoal(0, new WitherDoNothingGoal(this), true);
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new WitherRandomAttackPosGoal(this, 1.0f, 40, 10, 7), true);
    mGoalSelector.addGoal(5, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(7, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new WitherTargetHighestDamage(this), true);
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(
        3, new NearestAttackableTargetGoal(this, typeid(Mob), 0, false, false, LIVING_ENTITY_SELECTOR, true, false),
        true);
}

PathNavigation* WitherBoss::createNavigation(Level* level)
{
    FlyingPathNavigation* navigation = new FlyingPathNavigation(this, level);
    navigation->setCanOpenDoors(false);
    navigation->setCanFloat(true);
    navigation->setCanPassDoors(true);
    return navigation;
}

boost::shared_ptr<LivingEntity> WitherBoss::getTarget()
{
    if (mLevel->mIsClientSide && getAlternativeTarget(0) != getId())
    {
        boost::shared_ptr<Entity> target = mLevel->getEntity(getAlternativeTarget(0));
        if (target != NULL)
        {
            bool isLiving = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType());
            if (isLiving)
                return boost::static_pointer_cast<LivingEntity>(target);
        }
    }
    return Mob::getTarget();
}

const SoundEvent* WitherBoss::getAmbientSound()
{
    return SoundEvents::WITHER_AMBIENT;
}

void WitherBoss::checkDespawn()
{
    mNoActionTime = 0;
}

void WitherBoss::customServerAiStep()
{
    if (mBossEvent != NULL)
        mBossEvent->setPercent(getHealth() / getMaxHealth());

    if (mSpawningFrames > 0)
        return;

    if ((mOnGround || isInWater()) && mDiving)
    {
        mLevel->explode(shared_from_this(), mX, mY + getEyeHeight(), mZ, 7.0f, false,
                        mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING), false, false, false, -1.0f);
        mDiving = false;
        mPauseTicks = 80;

        if (mLevel->getDifficulty() != Difficulty::EASY)
        {
            for (int i = 0; i < 3; ++i)
            {
                int error;
                boost::shared_ptr<Entity> entity = SpawnEggItem::spawnMobAt(
                    mLevel, EntityIO::getKey(EntityType::WITHER_SKELETON), mX, mY, mZ, &error, L"");
                if (entity != NULL)
                {
                    boost::shared_ptr<WitherSkeleton> witherSkeleton = boost::static_pointer_cast<WitherSkeleton>(
                        entity);
                    if (error == 0)
                    {
                        witherSkeleton->setWitherParent(boost::static_pointer_cast<WitherBoss>(shared_from_this()));
                        witherSkeleton->mInvulnerableTime = 60;
                    }
                }
            }
        }
    }

    Mob::customServerAiStep();

    if (mHeadShotCooldown > 0)
        --mHeadShotCooldown;

    if (getTarget() != NULL && getTarget()->getId() != getAlternativeTarget(0))
        setAlternativeTarget(0, getTarget()->getId());
    else
        setAlternativeTarget(0, getId());

    if (mShotTicks > 0 && --mShotTicks == 0)
    {
        if (canShoot() && getTarget() != NULL)
        {
            if (getNavigation() != NULL)
                getNavigation()->stop();
            performRangedAttack(0, getTarget());
        }
        mShotTicks = mFireRate;
    }

    for (int head = 1; head < 3; ++head)
    {
        if (mTickCount >= mNextHeadUpdate[head - 1] && canShoot())
        {
            mHeadShotCooldown = mFireRate;
            mNextHeadUpdate[head - 1] = mTickCount + 10 + mRandom->nextInt(10);

            const Difficulty* difficulty = mLevel->getDifficulty();
            if ((difficulty == Difficulty::NORMAL || difficulty == Difficulty::HARD)
                && ++mIdleHeadUpdates[head - 1] > 15)
            {
                float x = mRandom->nextFloat((float)mX - 10.0f, (float)mX + 10.0f);
                float y = mRandom->nextFloat((float)mY - 5.0f, (float)mY + 5.0f);
                float z = mRandom->nextFloat((float)mZ - 10.0f, (float)mZ + 10.0f);
                if (head == 1)
                    performRangedAttack(1, x, y, z, true);
                mIdleHeadUpdates[head - 1] = 0;
            }

            int targetId = getAlternativeTarget(head);
            if (targetId != getId())
            {
                boost::shared_ptr<Entity> target = mLevel->getEntity(targetId);
                bool lostTarget = true;
                if (target != NULL && target->isAlive()
                    && distanceToSqr(target) <= (double)(mTargetRange * mTargetRange) && canSee(target))
                {
                    lostTarget = false;
                }

                if (lostTarget)
                {
                    setAlternativeTarget(head, getId());
                }
                else
                {
                    bool invulnerablePlayer = false;
                    const bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType());
                    if (isPlayer)
                    {
                        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(target);
                        if (player->mAbilities.mInvulnerable)
                            invulnerablePlayer = true;
                    }

                    if (invulnerablePlayer)
                    {
                        setAlternativeTarget(head, getId());
                    }
                    else
                    {
                        const bool isLiving = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType());
                        if (isLiving && (difficulty == Difficulty::NORMAL || difficulty == Difficulty::HARD))
                        {
                            performRangedAttack(head, boost::static_pointer_cast<LivingEntity>(target));
                            mNextHeadUpdate[head - 1] = mTickCount + 40 + mRandom->nextInt(20);
                            mIdleHeadUpdates[head - 1] = 0;
                        }
                    }
                }
            }
            else
            {
                std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(
                    typeid(Player), mBoundingBox->grow(mTargetRange, 8.0, mTargetRange));
                for (int attempt = 0; attempt < 10 && !players->empty(); ++attempt)
                {
                    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(
                        players->at(mRandom->nextInt(players->size())));
                    if (player->hasCategory(EntityCategory::eEntityCategory_Player) && !player->mAbilities.mInstabuild
                        && getTarget() != player)
                    {
                        setAlternativeTarget(head, player->getId());
                        break;
                    }
                    players->erase(std::remove(players->begin(), players->end(), player), players->end());
                }
                delete players;
            }
        }

        boost::shared_ptr<LivingEntity> target = getTarget();
        if (target != NULL)
            setAlternativeTarget(0, target->getId());
        else
            setAlternativeTarget(0, getId());

        if (mDestroyBlocksTick > 0 && --mDestroyBlocksTick == 0)
            destroyBlocks(mLevel, mBoundingBox, 1);
    }

    if (mChargeTimer > 0 && --mChargeTimer == 0)
    {
        if (!isPowered())
        {
            mWantsToMove = true;
        }
        else
        {
            mChargeDelay = 20;
            boost::shared_ptr<LivingEntity> target = getTarget();
            if (target != NULL)
            {
                delete mChargeDirection;
                mChargeDirection = Vec3::newPermanent(target->getPos(1.0f)->subtract(getPos(1.0f))->normalize());
            }
        }
    }

    if (mCharging)
    {
        destroyBlocks(mLevel, mBoundingBox, 2);
        std::vector<boost::shared_ptr<Entity> >* mobs = mLevel->getEntitiesOfClass(typeid(Mob),
                                                                                   mBoundingBox->grow(1.0, 4.0, 1.0));
        for (unsigned int i = 0; i < mobs->size(); ++i)
        {
            boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(mobs->at(i));
            if (mob != shared_from_this())
            {
                DamageSource* source = DamageSource::CreateMobAttack(
                    boost::static_pointer_cast<LivingEntity>(shared_from_this()));
                mob->hurt(source, 15.0f);
                delete source;
            }
        }
        delete mobs;
    }

    if (mChargeDelay > 0 && --mChargeDelay == 0)
    {
        boost::shared_ptr<LivingEntity> target = getTarget();
        if (target != NULL)
        {
            mChargeTicks = 10;
            mCharging = true;
        }
    }

    if (mTickCount % 20 == 0)
    {
        std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(
            typeid(Player), mBoundingBox->grow(50.0, 50.0, 50.0));
        delete players;
    }

    mEntityData->set(DATA_CHARGE_ATTACK, mChargeDelay != 0 || mCharging, true);
}

MobGroupData* WitherBoss::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString)
{
    MobGroupData* result = Mob::finalizeMobSpawn(difficulty, data, dataString);

    setSize(1.0f, 3.5f);
    setAerialAttack(true);
    mSpawningFrames = 200;
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.6f);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(70.0);
    makeInvulnerable();

    if (mLevel->getDifficulty() == Difficulty::EASY)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(getMaxHealth() * 0.5);
        mInitialShieldHealth /= 2;
    }
    else if (mLevel->getDifficulty() == Difficulty::NORMAL)
    {
        getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(getMaxHealth() * 0.75);
        mInitialShieldHealth = (int)(mInitialShieldHealth * 0.75f);
    }

    mMaxHealth = (int)getMaxHealth();
    setHealth(1.0f);
    mLastHealthInterval = (int)getMaxHealth();
    mPhaseHealth = (int)getMaxHealth() / 2;
    mHealthInterval = (int)getMaxHealth() / 6;
    return result;
}

int WitherBoss::getDimension()
{
    return mDimension;
}

void WitherBoss::makeInvulnerable()
{
    mInvulnerableTime = 200;
    setInvulnerableTicks(200);
}

void WitherBoss::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    performRangedAttack(0, target);
}

void WitherBoss::performRangedAttack(int head, boost::shared_ptr<LivingEntity> target)
{
    if (head == 0)
    {
        mTicksSinceShot = 0;
        ++mShotCount;
        bool dangerous = (mShotCount & 3) == 0;
        performRangedAttack(0, target->mX, target->mY + target->getEyeHeight() * 0.5, target->mZ,
                            (mShotCount & 3) == 0);
        if (dangerous)
        {
            mChargeTimer = mChargeInterval;
            if (!getAerialAttack())
            {
                mSkipNextCharge = !mSkipNextCharge;
                if (!mSkipNextCharge)
                {
                    mChargeTimer = 0;
                    mShotDelay = 60;
                }
            }
        }
    }
    else
    {
        performRangedAttack(head, target->mX, target->mY + target->getEyeHeight() * 0.5, target->mZ, false);
    }
}

void WitherBoss::performRangedAttack(int head, double x, double y, double z, bool dangerous)
{
    mLevel->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_WITHER_BOSS_SHOOT, BlockPos(mX, mY, mZ), 0);

    double headX = getHeadX(head);
    double headY = getHeadY(head);
    double headZ = getHeadZ(head);
    double xd = x - headX;
    double yd = y - headY;
    double zd = z - headZ;

    boost::shared_ptr<WitherSkull> skull(
        new WitherSkull(mLevel, boost::static_pointer_cast<LivingEntity>(shared_from_this()), xd, yd, zd));
    if (dangerous)
        skull->setDangerous(true);
    skull->mY = headY;
    skull->mX = headX;
    skull->mZ = headZ;
    mLevel->addEntity(skull);
}

bool WitherBoss::isAggressive()
{
    return false;
}

void WitherBoss::setAggressive(bool aggressive)
{
}

double WitherBoss::getHeadX(int head)
{
    if (head < 1)
        return mX;
    return Mth::cos((mYBodyRot + (float)(180 * head - 180)) * Mth::DEG_TO_RAD) * 1.3 + mX;
}

double WitherBoss::getHeadY(int head)
{
    if (head < 1)
        return mY + 3.0;
    return mY + 2.2;
}

double WitherBoss::getHeadZ(int head)
{
    if (head < 1)
        return mZ;
    return Mth::sin((mYBodyRot + (float)(180 * head - 180)) * Mth::DEG_TO_RAD) * 1.3 + mZ;
}

float WitherBoss::rotlerp(float from, float to, float maxDelta)
{
    float delta = Mth::wrapDegrees(to - from);
    if (delta > maxDelta)
        delta = maxDelta;
    if (delta < -maxDelta)
        delta = -maxDelta;

    float result = from + delta;
    if (result >= 180.0f)
        result -= 360.0f;
    if (result < -180.0f)
        result += 360.0f;
    return result;
}

void WitherBoss::setAerialAttack(bool aerialAttack)
{
    mEntityData->set(DATA_AERIAL_ATTACK, aerialAttack, true);
}

bool WitherBoss::getAerialAttack() const
{
    return mEntityData->get(DATA_AERIAL_ATTACK);
}

int WitherBoss::getInvulnerableTicks()
{
    return mEntityData->get(DATA_ID_INV);
}

void WitherBoss::setInvulnerableTicks(int ticks)
{
    mEntityData->set(DATA_ID_INV, ticks, true);
}

int WitherBoss::getAlternativeTarget(int head)
{
    return mEntityData->get(DATA_TARGETS[head]);
}

void WitherBoss::setAlternativeTarget(int head, int target)
{
    if (head != 0)
    {
        if (target != getId())
        {
            if (getAlternativeTarget(0) == target || getAlternativeTarget(1) == target
                || getAlternativeTarget(2) == target)
            {
                return;
            }
        }
    }
    else
    {
        if (target != getId() && getAlternativeTarget(1) == target)
            setAlternativeTarget(1, getId());
        if (target != getId() && getAlternativeTarget(2) == target)
            setAlternativeTarget(2, getId());
    }
    mEntityData->set(DATA_TARGETS[head], target, true);
}

bool WitherBoss::isPowered()
{
    return getHealth() <= getMaxHealth() * 0.5f;
}

void WitherBoss::generateParticles()
{
    if (!mLevel->mIsClientSide)
        return;

    if (getInvulnerableTicks() > 0)
    {
        for (int i = 0; i < 3; ++i)
        {
            mLevel->addParticle(PARTICLE_MOB_SPELL, mX + mRandom->nextGaussian() * 1.0,
                                mY + (float)(mRandom->nextFloat() * 3.3f), mZ + mRandom->nextGaussian() * 1.0, 0.7f,
                                0.7f, 0.9f, arrayWithLength<int>());
        }
    }

    bool powered = isPowered();
    for (int head = 0; head < 3; ++head)
    {
        double headX = getHeadX(head);
        double headY = getHeadY(head);
        double headZ = getHeadZ(head);
        mLevel->addParticle(PARTICLE_SMOKE, headX + mRandom->nextGaussian() * 0.3f,
                            headY + mRandom->nextGaussian() * 0.3f, headZ + mRandom->nextGaussian() * 0.3f, 0.0, 0.0,
                            0.0, arrayWithLength<int>());
        if (powered && mLevel->mRandom->nextInt(4) == 0)
        {
            mLevel->addParticle(PARTICLE_MOB_SPELL, headX + mRandom->nextGaussian() * 0.3f,
                                headY + mRandom->nextGaussian() * 0.3f, headZ + mRandom->nextGaussian() * 0.3f, 0.7f,
                                0.7f, 0.5, arrayWithLength<int>());
        }
    }
}

bool WitherBoss::IsChargeAttacking()
{
    return mEntityData->get(DATA_CHARGE_ATTACK);
}

bool WitherBoss::canShoot()
{
    return mChargeTimer == 0 && !mWantsToMove && !mCharging && mChargeDelay == 0 && mShotDelay < 1;
}

std::map<boost::shared_ptr<ServerPlayer>, int>& WitherBoss::getPlayerParty()
{
    return mBossEvent->getPartyPlayers();
}

void WitherBoss::changePhase()
{
    --mPhase;
    mMaxHealth -= (int)getMaxHealth() - mPhaseHealth;
    setAerialAttack(false);
    mDiving = true;
    mWantsToMove = false;
    mFireRate = 20;
    mChargeInterval <<= 1;
}

float WitherBoss::getHeadYRot(int head)
{
    return mYRotHeads[head];
}

float WitherBoss::getHeadXRot(int head)
{
    return mXRotHeads[head];
}

float WitherBoss::getOverlayAlpha()
{
    return mOverlayAlpha;
}

float WitherBoss::getSwellAmount(float partialTicks) const
{
    float swell = mSwellAmount;
    float old = mOldSwellAmount;
    return ((swell - old) * partialTicks + old) / 28.0f;
}

bool WitherBoss::wantsToMove()
{
    return mWantsToMove;
}

void WitherBoss::setWantsToMove(bool wantsToMove)
{
    mWantsToMove = wantsToMove;
}

void WitherBoss::setIsPathing(bool isPathing)
{
    mIsPathing = isPathing;
}

void WitherBoss::setShotDelay(int shotDelay)
{
    mShotDelay = shotDelay;
}

bool WitherBoss::IsSpawning() const
{
    return mSpawningFrames > 0;
}

void WitherBoss::destroyBlocks(Level* level, const AABB* area, int radius)
{
    if (!level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
        return;

    bool destroyed = false;
    for (int x = (int)area->mMinX - radius; x <= (int)area->mMaxX + radius; ++x)
    {
        for (int z = (int)area->mMinZ - radius; z <= (int)area->mMaxZ + radius; ++z)
        {
            for (int y = (int)area->mMinY - radius; y <= (int)area->mMaxY + radius; ++y)
            {
                BlockPos pos(x, y, z);
                const BlockState* state = level->getBlockState(pos);
                if (state->getMaterial() == Material::AIR)
                    continue;

                if (canDestroy(state->getBlock()))
                {
                    destroyed = level->removeBlock(pos) || destroyed;
                    state->getBlock()->spawnResources(level, pos, state, 1);
                }
            }
        }
    }

    if (destroyed)
        playSound(SoundEvents::WITHER_BREAK_BLOCK, 1.0f, 1.0f);
}

WitherBoss::WitherDoNothingGoal::WitherDoNothingGoal(WitherBoss* wither) : Goal(), mWither(wither)
{
    setRequiredControlFlags(7);
}

WitherBoss::WitherDoNothingGoal::~WitherDoNothingGoal()
{
}

bool WitherBoss::WitherDoNothingGoal::canUse()
{
    return mWither->getInvulnerableTicks() > 0;
}
