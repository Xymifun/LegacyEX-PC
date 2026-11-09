#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/Attribute.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/ModifiableAttributeMap.hpp"
#include "game/core/CombatRules.hpp"
#include "game/core/CombatTracker.hpp"
#include "game/core/Drowned.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/ExperienceOrb.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/InsomniaComponent.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/BubbleColumnSegment.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/Stat.hpp"
#include "game/core/Team.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/ai/PushableBySelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/FrostWalkerEnchantment.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundAnimatePacket.hpp"
#include "game/network/ClientboundSetEquippedItemPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/ClientboundTakeItemEntityPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TrapDoorBlock.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModViews.hpp"
#endif


const AttributeModifier* LivingEntity::SPEED_MODIFIER_SPRINTING =
    (new AttributeModifier(eMODIFIER_ID_SprintingSpeedBoost, 0.30000001192092896, 2))->setSerialize(false);
const EntityDataAccessor<unsigned char>* LivingEntity::DATA_LIVING_ENTITY_FLAGS;
const EntityDataAccessor<float>* LivingEntity::DATA_HEALTH_ID;
const EntityDataAccessor<int>* LivingEntity::DATA_EFFECT_COLOR_ID;
const EntityDataAccessor<bool>* LivingEntity::DATA_EFFECT_AMBIENCE_ID;
const EntityDataAccessor<int>* LivingEntity::DATA_ARROW_COUNT_ID;

void LivingEntity::staticCtor()
{
    DATA_LIVING_ENTITY_FLAGS = SynchedEntityData::defineId(EntityType::LIVING_ENTITY, EntityType::ENTITY,
                                                           EntityDataSerializers::BYTE);
    DATA_HEALTH_ID = SynchedEntityData::defineId(EntityType::LIVING_ENTITY, EntityType::ENTITY,
                                                 EntityDataSerializers::FLOAT);
    DATA_EFFECT_COLOR_ID = SynchedEntityData::defineId(EntityType::LIVING_ENTITY, EntityType::ENTITY,
                                                       EntityDataSerializers::INT);
    DATA_EFFECT_AMBIENCE_ID = SynchedEntityData::defineId(EntityType::LIVING_ENTITY, EntityType::ENTITY,
                                                          EntityDataSerializers::BOOLEAN);
    DATA_ARROW_COUNT_ID = SynchedEntityData::defineId(EntityType::LIVING_ENTITY, EntityType::ENTITY,
                                                      EntityDataSerializers::INT);
}

LivingEntity::LivingEntity(Level* level) : Entity(level, true)
{
    MemSect(56);
    _init();
    MemSect(0);

    mBlocksBuilding = true;
    mRotA = (float)(Math::random() + 1.0) * 0.01f;
    setPos(mX, mY, mZ);
    mTimeOffs = (float)Math::random() * 12398.0f;
    mYRot = (float)(Math::random() * 3.1415927410125732 * 2.0);
    mYHeadRot = mYRot;
    mSwingCount = 0;
    mMaxUpStep = 0.6f;
}

LivingEntity::~LivingEntity()
{
    for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
         it != mActiveEffects.end(); ++it)
    {
        delete it->second;
    }

    delete mAttributes;
    delete mCombatTracker;
    delete[] mLastHandItems.items.data;
    delete[] mLastArmorItems.items.data;

    if (mLastDamageSource)
    {
        delete mLastDamageSource;
        mLastDamageSource = NULL;
    }

    delete mInsomniaComponent;
}

void LivingEntity::_init()
{
    mAttributes = NULL;
    mCombatTracker = new CombatTracker(this);
    mLastHandItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mLastArmorItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(4, ItemInstance::EMPTY);
    mSwinging = false;
    mSwingTime = 0;
    mRemoveArrowTime = 0;
    mHurtTime = 0;
    mHurtDuration = 0;
    mHurtDir = 0.0f;
    mDeathTime = 0;
    mOAttackAnim = 0.0f;
    mAttackAnim = 0.0f;
    mAnimationSpeedOld = 0.0f;
    mAnimationSpeed = 0.0f;
    mAnimationPosition = 0.0f;
    mInvulnerableDuration = 20;
    mOBob = 0.0f;
    mBob = 0.0f;
    mTimeOffs = 0.0f;
    mRotA = 0.0f;
    mYBodyRot = 0.0f;
    mYBodyRotO = 0.0f;
    mYHeadRot = 0.0f;
    mYHeadRotO = 0.0f;
    mFlyingSpeed = 0.02f;
    mGlideDistance = 0.0;
    mThermalLiftSpeed = 0.0;
    mThermalLerping = false;
    mThermalLerpTicks = 0;
    mThermalTargetLiftSpeed = 0.0;
    mThermalLiftRising = true;
    mThermalCeiling = 0.0;
    mInThermalArea = false;
    mElytraSpeedBoostActive = false;
    mElytraSpeedBoost = 0.0;
    mThermalArea = NULL;
    mThermalAreaChanged = true;
    mThermalAreasChecked = false;
    mLastHurtByPlayer = boost::shared_ptr<Player>();
    mHurtByPlayerOnPeaceful = false;
    mLastHurtByPlayerTime = 0;
    mDead = false;
    mNoActionTime = 0;
    mORun = 0.0f;
    mRun = 0.0f;
    mAnimStep = 0.0f;
    mAnimStepO = 0.0f;
    mRotOffs = 0.0f;
    mDeathScore = 0;
    mLastHurt = 0.0f;
    mJumping = false;
    mXxa = 0.0f;
    mYya = 0.0f;
    mZza = 0.0f;
    mYRota = 0.0f;
    mLerpSteps = 0;
    mLerpX = 0.0;
    mLerpY = 0.0;
    mLerpZ = 0.0;
    mLerpYRot = 0.0;
    mLerpXRot = 0.0;
    mEffectsDirty = false;
    mLastHurtByMob.reset();
    mLastHurtByMobTimestamp = 0;
    mLastHurtMob.reset();
    mLastHurtMobTimestamp = 0;
    mSpeed = 0.0f;
    mNoJumpDelay = 0;
    mAbsorptionAmount = 0.0f;
    mLastPos = BlockPos(0, 0, 0);
    mUseItem = ItemInstance::EMPTY;
    mUseItemRemaining = 0;
    mLastDamageSource = NULL;
    mFallFlyingStateDisabled = false;
    mElytraCollisionDistance = 0.0;
    mDeathMessageId = 0;
    mDeathMessage = L"";
    mInsomniaComponent = NULL;
    mLightCacheTicks = 5;
    mLightSampleMinX = 0;
    mLightSampleMinY = 0;
    mLightSampleMinZ = 0;
    mLightSampleMaxX = 0;
    mLightSampleMaxY = 0;
    mLightSampleMaxZ = 0;
    mLightCacheValid = false;
    mLastLightColor = 0;
    mSwimSoundPlayed = false;
    mWaterTravelSpeedFactor = 1.0f;
    mWaterTravelCheckTicks = 60;
}

void LivingEntity::fjDerivedCtorCalls()
{
    Entity::fjDerivedCtorCalls();

    registerAttributes();
    setHealth(getMaxHealth());
}

void LivingEntity::ResetThermalVariables()
{
    mThermalLerping = false;
    mElytraSpeedBoost = 0.0;
    mThermalLerpTicks = 0;
    mThermalAreasChecked = true;
    mInThermalArea = false;
    mThermalLiftSpeed = 0.0;
    mZd = 0.0;
    mElytraProximityWarning = false;
    mXd = 0.0;
    mYd = 0.0;
    mElytraSpeedBoostActive = false;
    mFallDistance = 0.0f;
    SetLiftForceModifier(1.0);
    mGlideDistance = 0.0;
}

bool LivingEntity::isInterpolating() const
{
    return mLerpSteps > 0;
}

double LivingEntity::getLerpToYRot() const
{
    return mLerpYRot;
}

double LivingEntity::getLerpToXRot() const
{
    return mLerpXRot;
}

void LivingEntity::kill()
{
    hurt(DamageSource::OUT_OF_WORLD, 3.4028234663852886e+38f);
}

void LivingEntity::defineSynchedData()
{
    mEntityData->createDateItem(DATA_LIVING_ENTITY_FLAGS, (unsigned char)0);
    mEntityData->createDateItem(DATA_EFFECT_COLOR_ID, 0);
    mEntityData->createDateItem(DATA_EFFECT_AMBIENCE_ID, false);
    mEntityData->createDateItem(DATA_ARROW_COUNT_ID, 0);
    mEntityData->createDateItem(DATA_HEALTH_ID, 1.0f);
}

void LivingEntity::registerAttributes()
{
    getAttributes()->registerAttribute(SharedMonsterAttributes::MAX_HEALTH);
    getAttributes()->registerAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE);
    getAttributes()->registerAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ARMOR);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ARMOR_TOUGHNESS);
}

void LivingEntity::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
    if (!isInWater())
    {
        updateInWaterState();
    }

    if ((isInWater() || hasEffect(MobEffects::SLOW_FALLING)) && !isVehicle())
    {
        mFallDistance = 0.0f;
    }

    if (!mLevel->mIsClientSide && mFallDistance > 3.0f && onGround)
    {
        const BlockState* state = mLevel->getBlockState(mTickCache, pos);
        float excess = ceilf(mFallDistance - 3.0f);

        if (block->mMaterial != Material::AIR)
        {
            float scale = excess / 15.0f + 0.2f;
            float capped = scale > 2.5f ? 2.5f : scale;
            int count = (int)(150.0 * capped);
            int blockData = Block::getIdWithData(state);
            arrayWithLength<int> particleData(&blockData, 1);

            static_cast<ServerLevel*>(mLevel)->sendParticles(PARTICLE_BLOCK_DUST, mX, mY, mZ, count, 0.0, 0.0, 0.0,
                                                             0.15f, particleData);
        }
    }

    Entity::checkFallDamage(y, onGround, block, pos);
}

InsomniaComponent* LivingEntity::getInsomniaComponent() const
{
    return mInsomniaComponent;
}

bool LivingEntity::isWaterMob()
{
    return false;
}

bool LivingEntity::breathesWater()
{
    return false;
}

bool LivingEntity::breathesAir()
{
    return true;
}

bool LivingEntity::breathesLava()
{
    return false;
}

bool LivingEntity::breathesSolids()
{
    return false;
}

int LivingEntity::GetSuffocateTime()
{
    return 0;
}

float LivingEntity::GetInhaleTime()
{
    return 0.0f;
}

int LivingEntity::GetAirRegenPerTick()
{
    float inhaleTime = GetInhaleTime();
    int airRegen = getMaxAirSupply();

    if (fabsf(inhaleTime) > 0.0001f)
    {
        airRegen = Math::round((float)airRegen / (inhaleTime * 20.0f));
    }

    return airRegen;
}

bool LivingEntity::canBreathe()
{
    if (hasEffect(MobEffects::WATER_BREATHING) || hasEffect(MobEffects::CONDUIT_POWER))
    {
        return true;
    }

    BlockPos pos(mX, mY + (double)getEyeHeight(), mZ);
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);
    Block* block = state->getBlock();
    Material* material = state->getMaterial();
    Material* fluidMaterial = mLevel->getExtraBlockState(pos)->getMaterial();

    if (block == Blocks::air || (!material->isSolid() && !material->isLiquid() && !fluidMaterial->isLiquid()))
    {
        return breathesAir();
    }

    if (block == Blocks::bubble_column_segment)
    {
        return breathesAir();
    }

    if (!material->isLiquid() && !fluidMaterial->isLiquid())
    {
        if (material != Material::LAVA && fluidMaterial != Material::LAVA)
        {
            if (state->isSolidBlockingBlock() && mLevel->isConsideredSolidBlock(pos) && isInWall())
            {
                return breathesSolids();
            }

            return breathesAir();
        }
    }
    else if (material != Material::LAVA && fluidMaterial != Material::LAVA)
    {
        if (!breathesWater() && isUnderLiquid(Material::WATER))
        {
            return false;
        }

        return true;
    }

    return breathesLava();
}

bool LivingEntity::generatesBubbles()
{
    return true;
}

void LivingEntity::baseTick()
{
    if (gApp.IsAppPaused())
    {
        return;
    }

    mOAttackAnim = mAttackAnim;

    if (isAlive() && isInWaterRainOrBubble())
    {
        clearFire();
    }

    Entity::baseTick();

    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType());

    if (isAlive())
    {
        if (isInWall())
        {
            hurt(DamageSource::IN_WALL, 1.0f);
        }
        else if (isPlayer && !mLevel->getWorldBorder()->isWithinBounds(mBoundingBox))
        {
            double distance = mLevel->getWorldBorder()->getDistanceToBorder(shared_from_this())
                              + mLevel->getWorldBorder()->getDamageSafeZone();

            if (distance < 0.0)
            {
                if (mLevel->getWorldBorder()->getDamagePerBlock() > 0.0)
                {
                    hurt(DamageSource::IN_WALL,
                         std::max(1.0f, (float)floor(-distance * mLevel->getWorldBorder()->getDamagePerBlock())));
                }
            }
        }
    }

    if (isFireImmune() || mLevel->mIsClientSide)
    {
        clearFire();
    }

    bool instabuildPlayer = false;

    if (isPlayer && boost::static_pointer_cast<Player>(shared_from_this())->mAbilities.mInstabuild)
    {
        instabuildPlayer = true;
    }

    if (isAlive())
    {
        bool canBreatheNow = instabuildPlayer || canBreathe();
        int airSupply = getAirSupply();
        int maxAirSupply = getMaxAirSupply();

        if (canBreatheNow)
        {
            if (airSupply < maxAirSupply)
            {
                airSupply = Mth::clamp(airSupply + GetAirRegenPerTick(), 0, maxAirSupply);
            }
        }
        else
        {
            if (isUnderLiquid(Material::WATER) && generatesBubbles() && canCreateParticles())
            {
                Vec3* eyePos = getAttachPos(ENTITY_LOCATION_BREATHING_POINT, 0.0f);
                mLevel->addParticle(PARTICLE_BUBBLE, eyePos->x, eyePos->y, eyePos->z, mXd, mYd, mZd,
                                    arrayWithLength<int>());
            }

            if (!mLevel->mIsClientSide || isPlayer)
            {
                airSupply = decreaseAirSupply(airSupply);
            }

            if (airSupply <= GetSuffocateTime())
            {
                if (isUnderLiquid(Material::WATER) && canCreateParticles() && isPlayer)
                {
                    for (int i = 0; i < 8; ++i)
                    {
                        float xo = mRandom->nextFloat() - mRandom->nextFloat();
                        float yo = mRandom->nextFloat() - mRandom->nextFloat();
                        float zo = mRandom->nextFloat() - mRandom->nextFloat();
                        mLevel->addParticle(PARTICLE_BUBBLE, mX + xo, mY + yo, mZ + zo, mXd, mYd, mZd,
                                            arrayWithLength<int>());
                    }
                }

                airSupply = 0;
                hurt(DamageSource::DROWN, 2.0f);
            }

            if (!isPlayer && isUnderLiquid(Material::WATER) && canCreateParticles() && breathesAir()
                && airSupply < getMaxAirSupply())
            {
                for (int i = 0; i < 8; ++i)
                {
                    Vec3* eyePos = getAttachPos(ENTITY_LOCATION_BREATHING_POINT, 0.0f);
                    mLevel->addParticle(PARTICLE_BUBBLE, (float)eyePos->x + mRandom->nextGaussianFloat(),
                                        (float)eyePos->y + mRandom->nextGaussianFloat(),
                                        (float)eyePos->z + mRandom->nextGaussianFloat(), mXd, mYd, mZd,
                                        arrayWithLength<int>());
                }
            }

            if (!mLevel->mIsClientSide && isPassenger() && getVehicle()->instanceof(EntityType::LIVING_ENTITY)
                && !getVehicle()->instanceof(EntityType::DOLPHIN)
                && !getVehicle()->instanceof(EntityType::SKELETON_HORSE))
            {
                stopRiding();
            }
        }

        if (Minecraft::InMiniGame(eMiniGameId_Battle, false) && isInvulnerable())
        {
            airSupply = 300;
        }

        setAirSupply(airSupply);
    }

    BlockPos pos(this);

    if (!pos.equals(mLastPos))
    {
        mLastPos = pos;
        onChangedBlock(pos);
    }

    mOBob = mBob;

    if (mHurtTime > 0)
    {
        --mHurtTime;
    }

    if (mInvulnerableTime > 0 && !instanceof(EntityType::SERVER_PLAYER))
    {
        --mInvulnerableTime;
    }

    if (getHealth() <= 0.0f)
    {
        tickDeath();
    }

    if (mLastHurtByPlayerTime > 0)
    {
        --mLastHurtByPlayerTime;
    }
    else if (mLastHurtByPlayer != NULL)
    {
        mLastHurtByPlayer.reset();
    }

    boost::shared_ptr<LivingEntity> lastHurtMob = mLastHurtMob.lock();

    if (lastHurtMob && !lastHurtMob->isAlive())
    {
        mLastHurtMob.reset();
    }

    boost::shared_ptr<LivingEntity> lastHurtByMob = getLastHurtByMob();

    if (lastHurtByMob)
    {
        if (!lastHurtByMob->isAlive())
        {
            setLastHurtByMob(boost::shared_ptr<LivingEntity>());
        }
        else if (mTickCount - mLastHurtByMobTimestamp > 100)
        {
            setLastHurtByMob(boost::shared_ptr<LivingEntity>());
        }
    }

    tickEffects();

    mAnimStepO = mAnimStep;
    mYBodyRotO = mYBodyRot;
    mYHeadRotO = mYHeadRot;
    mYRotO = mYRot;
    mXRotO = mXRot;
}

void LivingEntity::onChangedBlock(const BlockPos& pos)
{
    int level = EnchantmentHelper::getEnchantmentLevel(Enchantments::FROST_WALKER,
                                                       boost::static_pointer_cast<LivingEntity>(shared_from_this()));

    if (level > 0)
    {
        FrostWalkerEnchantment::onEntityMoved(boost::static_pointer_cast<LivingEntity>(shared_from_this()), mLevel, pos,
                                              level);
    }
}

bool LivingEntity::isBaby()
{
    return false;
}

void LivingEntity::tickDeath()
{
    ++mDeathTime;

    if (mDeathTime == 20)
    {
        if (!mLevel->mIsClientSide && (mLastHurtByPlayerTime > 0 || isAlwaysExperienceDropper())
            && shouldDropExperience() && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT))
        {
            int reward = getExperienceReward(mLastHurtByPlayer);

            while (reward > 0)
            {
                int value = ExperienceOrb::getExperienceValue(reward);
                reward -= value;
                mLevel->addEntity(boost::shared_ptr<Entity>(new ExperienceOrb(mLevel, mX, mY, mZ, value)));
            }
        }

        remove();

        for (int i = 0; i < 20; ++i)
        {
            double xd = mRandom->nextGaussian() * 0.02;
            double yd = mRandom->nextGaussian() * 0.02;
            double zd = mRandom->nextGaussian() * 0.02;
            double x = mX + (double)(mRandom->nextFloat() * mBbWidth * 2.0f) - (double)mBbWidth;
            double y = mY + (double)(mRandom->nextFloat() * mBbHeight);
            double z = mZ + (double)(mRandom->nextFloat() * mBbWidth * 2.0f) - (double)mBbWidth;
            mLevel->addParticle(PARTICLE_EXPLODE, x, y, z, xd, yd, zd, arrayWithLength<int>());
        }
    }
}

bool LivingEntity::shouldDropExperience()
{
    return !isBaby();
}

int LivingEntity::decreaseAirSupply(int airSupply)
{
    int respiration = EnchantmentHelper::getRespiration(boost::static_pointer_cast<LivingEntity>(shared_from_this()));

    if (respiration > 0 && mRandom->nextInt(respiration + 1) > 0)
    {
        return airSupply;
    }

    return airSupply - 1;
}

int LivingEntity::getExperienceReward(boost::shared_ptr<Player> player)
{
    return 0;
}

bool LivingEntity::isAlwaysExperienceDropper()
{
    return false;
}

void LivingEntity::stopRiding()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();

    Entity::stopRiding();

    if (vehicle && vehicle != getVehicle()
        && (!mLevel->mIsClientSide
            || (eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType())
                && eTYPE_DERIVED_FROM(EntityType::BOAT, vehicle->GetType()))))
    {
        findStandUpPosition(vehicle);
    }
}

Random* LivingEntity::getRandom()
{
    return mRandom;
}

boost::shared_ptr<LivingEntity> LivingEntity::getLastHurtByMob()
{
    return mLastHurtByMob.lock();
}

int LivingEntity::getLastHurtByMobTimestamp()
{
    return mLastHurtByMobTimestamp;
}

void LivingEntity::setLastHurtByMob(boost::shared_ptr<LivingEntity> attacker)
{
    mLastHurtByMob = attacker;
    mLastHurtByMobTimestamp = mTickCount;
}

boost::shared_ptr<LivingEntity> LivingEntity::getLastHurtMob()
{
    return mLastHurtMob.lock();
}

int LivingEntity::getLastHurtMobTimestamp()
{
    return mLastHurtMobTimestamp;
}

void LivingEntity::setLastHurtMob(boost::shared_ptr<Entity> target)
{
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
    {
        mLastHurtMob = boost::static_pointer_cast<LivingEntity>(target);
    }
    else
    {
        mLastHurtMob.reset();
    }

    mLastHurtMobTimestamp = mTickCount;
}

int LivingEntity::getNoActionTime()
{
    return mNoActionTime;
}

void LivingEntity::playEquipSound(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->isEmpty())
    {
        return;
    }

    if (eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, GetType()))
    {
        Item* item = itemInstance->getItem();
        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);

        if (armorItem != NULL)
        {
            playSound(armorItem->getMaterial()->getEquipSound(), 1.0f, 1.0f);
            return;
        }

        const SoundEvent* sound = SoundEvents::ARMOR_EQUIP_GENERIC;

        if (item == Items::elytra)
        {
            sound = SoundEvents::ARMOR_EQUIP_ELYTRA;
        }

        playSound(sound, 1.0f, 1.0f);
        return;
    }

    if (itemInstance->getItem() == Items::elytra)
    {
        playSound(SoundEvents::ARMOR_EQUIP_ELYTRA, 1.0f, 1.0f);
    }
}

void LivingEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putFloat(std::wstring(L"Health"), getHealth());
    tag->putShort(std::wstring(L"HurtTime"), (short)mHurtTime);
    tag->putShort(std::wstring(L"HurtByTimestamp"), (short)mLastHurtByMobTimestamp);
    tag->putShort(std::wstring(L"DeathTime"), (short)mDeathTime);
    tag->putFloat(std::wstring(L"AbsorptionAmount"), getAbsorptionAmount());

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> itemInstance = getItemSlot(slot);

        if (!itemInstance->isEmpty())
        {
            mAttributes->removeAttributeModifiers(itemInstance->getAttributeModifiers(slot));
        }
    }

    tag->put(std::wstring(L"Attributes"), SharedMonsterAttributes::saveAttributes(getAttributes()));

    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> itemInstance = getItemSlot(slot);

        if (!itemInstance->isEmpty())
        {
            mAttributes->addAttributeModifiers(itemInstance->getAttributeModifiers(slot));
        }
    }

    if (!mActiveEffects.empty())
    {
        ListTag* effects = new ListTag();

        for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
             it != mActiveEffects.end(); ++it)
        {
            effects->add(it->second->save(new CompoundTag()));
        }

        tag->put(std::wstring(L"ActiveEffects"), effects);
    }

    tag->putBoolean(std::wstring(L"FallFlying"), isFallFlying());

    if (mInsomniaComponent != NULL)
    {
        mInsomniaComponent->addAdditionalSaveData(tag);
    }
}

void LivingEntity::readAdditionalSaveData(CompoundTag* tag)
{
    setAbsorptionAmount(tag->getFloat(std::wstring(L"AbsorptionAmount")));

    if (tag->contains(std::wstring(L"Attributes"), 9) && mLevel != NULL && !mLevel->mIsClientSide)
    {
        SharedMonsterAttributes::loadAttributes(getAttributes(), tag->getList(std::wstring(L"Attributes"), 10));
    }

    if (tag->contains(std::wstring(L"ActiveEffects"), 9))
    {
        ListTag* effects = tag->getList(std::wstring(L"ActiveEffects"), 10);

        for (int i = 0; i < effects->size(); ++i)
        {
            MobEffectInstance* effectInstance = MobEffectInstance::load(effects->getCompound(i));

            if (effectInstance != NULL)
            {
                mActiveEffects.insert(std::make_pair(effectInstance->getEffect(), effectInstance));
            }
        }
    }

    if (tag->contains(std::wstring(L"Health"), 99))
    {
        setHealth(tag->getFloat(std::wstring(L"Health")));
    }

    mHurtTime = tag->getShort(std::wstring(L"HurtTime"));
    mDeathTime = tag->getShort(std::wstring(L"DeathTime"));
    mLastHurtByMobTimestamp = tag->getInt(std::wstring(L"HurtByTimestamp"));

    if (tag->contains(std::wstring(L"Team"), 8))
    {
        std::wstring teamName = tag->getString(std::wstring(L"Team"));
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());

        mLevel->GetScoreboard()->addPlayerToTeam(player->mXuid, mLevel->GetScoreboard()->getPlayerTeam(teamName));
    }

    if (tag->getBoolean(std::wstring(L"FallFlying")))
    {
        setSharedFlag(FLAG_FALL_FLYING, true);
    }

    if (mInsomniaComponent != NULL)
    {
        mInsomniaComponent->readAdditionalSaveData(tag);
    }
}

void LivingEntity::tickEffects()
{
    unsigned int effectCount = mActiveEffects.size();
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();

    while (it != mActiveEffects.end())
    {
        MobEffectInstance* effectInstance = it->second;

        if (!effectInstance->tick(boost::static_pointer_cast<LivingEntity>(shared_from_this())))
        {
            if (mLevel->mIsClientSide)
            {
                ++it;
            }
            else
            {
                it = mActiveEffects.erase(it);
                onEffectRemoved(effectInstance);
                --effectCount;
                delete effectInstance;

                if (mActiveEffects.size() != effectCount)
                {
                    break;
                }
            }
        }
        else
        {
            if (effectInstance->getDuration() % 600 == 0)
            {
                onEffectUpdated(effectInstance, false);
            }

            ++it;
        }
    }

    if (mEffectsDirty)
    {
        if (!mLevel->mIsClientSide)
        {
            updateInvisibilityStatus();
        }

        mEffectsDirty = false;
    }

    int colour = mEntityData->get(DATA_EFFECT_COLOR_ID);
    bool ambient = mEntityData->get(DATA_EFFECT_AMBIENCE_ID);

    if (colour > 0)
    {
        bool spawn;

        if (isInvisible())
        {
            spawn = mRandom->nextInt(15) == 0;
        }
        else
        {
            spawn = mRandom->nextBoolean();
        }

        if (ambient)
        {
            spawn = spawn && mRandom->nextInt(5) == 0;
        }

        if (spawn)
        {
            double red = (double)((colour >> 16) & 255) / 255.0;
            double green = (double)((colour >> 8) & 255) / 255.0;
            double blue = (double)(colour & 255) / 255.0;

            mLevel->addParticle(
                ambient ? PARTICLE_MOB_SPELL_AMBIENT : PARTICLE_MOB_SPELL,
                mX + (mRandom->nextDouble() - 0.5) * (double)mBbWidth, mY + mRandom->nextDouble() * (double)mBbHeight,
                mZ + (mRandom->nextDouble() - 0.5) * (double)mBbWidth, red, green, blue, arrayWithLength<int>());
        }
    }
}

bool LivingEntity::areAllEffectsAmbient(std::vector<MobEffectInstance*>* effects)
{
    for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
    {
        if (!(*it)->isAmbient())
        {
            return false;
        }
    }

    return true;
}

void LivingEntity::updateInvisibilityStatus()
{
    if (mActiveEffects.empty())
    {
        mEntityData->set(DATA_EFFECT_AMBIENCE_ID, false, true);
        mEntityData->set(DATA_EFFECT_COLOR_ID, 0, true);
        setInvisible(false);
        setWeakened(false);
        return;
    }

    std::vector<MobEffectInstance*> effects;

    for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
         it != mActiveEffects.end(); ++it)
    {
        effects.push_back(it->second);
    }

    mEntityData->set(DATA_EFFECT_AMBIENCE_ID, areAllEffectsAmbient(&effects), true);
    mEntityData->set(DATA_EFFECT_COLOR_ID, PotionUtils::getColor(&effects), true);

    setInvisible(hasEffect(MobEffects::INVISIBILITY));
    setWeakened(hasEffect(MobEffects::WEAKNESS));
}

void LivingEntity::removeEffectParticles()
{
    mEntityData->set(DATA_EFFECT_AMBIENCE_ID, false, true);
    mEntityData->set(DATA_EFFECT_COLOR_ID, 0, true);
}

void LivingEntity::removeAllEffects()
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();

    while (it != mActiveEffects.end())
    {
        MobEffectInstance* effectInstance = it->second;
        it = mActiveEffects.erase(it);
        onEffectRemoved(effectInstance);
        delete effectInstance;
    }
}

std::vector<MobEffectInstance*>* LivingEntity::getActiveEffects()
{
    std::vector<MobEffectInstance*>* effects = new std::vector<MobEffectInstance*>();

    for (boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.begin();
         it != mActiveEffects.end(); ++it)
    {
        effects->push_back(it->second);
    }

    return effects;
}

bool LivingEntity::hasEffect(MobEffect* effect)
{
    return mActiveEffects.find(effect) != mActiveEffects.end();
}

MobEffectInstance* LivingEntity::getEffect(MobEffect* effect)
{
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.find(effect);

    if (it == mActiveEffects.end())
    {
        return NULL;
    }

    return it->second;
}

void LivingEntity::addEffect(MobEffectInstance* effectInstance, const boost::shared_ptr<Entity>& source)
{
    if (!canBeAffected(effectInstance))
    {
        return;
    }

    if (mActiveEffects.find(effectInstance->getEffect()) == mActiveEffects.end())
    {
        mActiveEffects[effectInstance->getEffect()] = effectInstance;
        onEffectAdded(effectInstance);
    }
    else
    {
        MobEffectInstance* existing = mActiveEffects[effectInstance->getEffect()];
        existing->update(effectInstance);
        onEffectUpdated(existing, true);
    }
}

void LivingEntity::addEffectNoUpdate(MobEffectInstance* effectInstance)
{
    if (!canBeAffected(effectInstance))
    {
        return;
    }

    if (mActiveEffects.find(effectInstance->getEffect()) != mActiveEffects.end())
    {
        mActiveEffects[effectInstance->getEffect()] = effectInstance;
        onEffectAdded(effectInstance);
    }
    else
    {
        mActiveEffects.find(effectInstance->getEffect())->second->update(effectInstance);
    }
}

bool LivingEntity::canBeAffected(MobEffectInstance* effectInstance)
{
    if (getMobType() == MOB_TYPE_UNDEAD
        && (effectInstance->getEffect() == MobEffects::REGENERATION
            || effectInstance->getEffect() == MobEffects::POISON))
    {
        return false;
    }

    return true;
}

bool LivingEntity::isInvertedHealAndHarm()
{
    return getMobType() == MOB_TYPE_UNDEAD;
}

MobEffectInstance* LivingEntity::removeEffectNoUpdate(MobEffect* effect)
{
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.find(effect);

    if (it == mActiveEffects.end())
    {
        return NULL;
    }

    MobEffectInstance* effectInstance = it->second;
    mActiveEffects.erase(it);

    return effectInstance;
}

void LivingEntity::removeEffect(MobEffect* effect)
{
    boost::unordered_map<MobEffect*, MobEffectInstance*>::iterator it = mActiveEffects.find(effect);

    if (it == mActiveEffects.end())
    {
        return;
    }

    MobEffectInstance* effectInstance = it->second;

    if (effectInstance != NULL)
    {
        onEffectRemoved(effectInstance);
        delete effectInstance;
    }

    mActiveEffects.erase(it);
}

void LivingEntity::onEffectAdded(MobEffectInstance* effectInstance)
{
    mEffectsDirty = true;

    if (!mLevel->mIsClientSide)
    {
        MobEffect* effect = effectInstance->getEffect();
        effect->addAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(),
                                      effectInstance->getAmplifier());
    }
}

void LivingEntity::onEffectUpdated(MobEffectInstance* effectInstance, bool reapply)
{
    mEffectsDirty = true;

    if (reapply && !mLevel->mIsClientSide)
    {
        MobEffect* effect = effectInstance->getEffect();
        effect->removeAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(),
                                         effectInstance->getAmplifier());
        effect->addAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(),
                                      effectInstance->getAmplifier());
    }
}

void LivingEntity::onEffectRemoved(MobEffectInstance* effectInstance)
{
    mEffectsDirty = true;

    if (!mLevel->mIsClientSide)
    {
        MobEffect* effect = effectInstance->getEffect();
        effect->removeAttributeModifiers(boost::static_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(),
                                         effectInstance->getAmplifier());
    }
}

void LivingEntity::heal(float amount)
{
    float health = getHealth();

    if (health > 0.0f)
    {
        setHealth(health + amount);
    }
}

float LivingEntity::getHealth()
{
    return mEntityData->get(DATA_HEALTH_ID);
}

void LivingEntity::setHealth(float health)
{
    mEntityData->set(DATA_HEALTH_ID, Mth::clamp(health, 0.0f, getMaxHealth()), true);
}


bool LivingEntity::hurt(const DamageSource* source, float damage)
{
    if ((isInvulnerableTo(source) && !source->isBypassInvul()) || gApp.IsAppPaused()
        || (mLevel->mIsClientSide && dynamic_cast<const EntityDamageSource*>(source) == NULL))
    {
        return false;
    }

    mNoActionTime = 0;

    if (getHealth() <= 0.0f)
    {
        return false;
    }

    if (source->isFire() && hasEffect(MobEffects::FIRE_RESISTANCE))
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()) && source == DamageSource::LAVA)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());
            player->awardStat(GenericStats::stayinFrosty(), GenericStats::param_stayinFrosty());
        }

        return false;
    }

    float initialDamage = damage;

    bool helmetHit = false;

    if (source == DamageSource::ANVIL || source == DamageSource::FALLING_BLOCK)
    {
        if (!getItemSlot(EquipmentSlot::HEAD)->isEmpty())
        {
            helmetHit = true;
        }
    }

    if (helmetHit)
    {
        getItemSlot(EquipmentSlot::HEAD)
            ->hurtAndBreak((int)(damage * 4.0f + mRandom->nextFloat() * damage * 2.0f),
                           boost::static_pointer_cast<LivingEntity>(shared_from_this()));
        damage *= 0.75f;
    }

    bool blocked = false;

    if (damage > 0.0f && isDamageSourceBlocked(source))
    {
        damage = (damage + 1.0f) * 0.5f;
        blocked = true;
    }

    mAnimationSpeed = 1.5f;

    bool freshHurt = true;

    if (!source->isBypassGraceInvul() && (float)mInvulnerableTime > (float)mInvulnerableDuration * 0.5f)
    {
        if (damage <= mLastHurt)
        {
            return false;
        }

        if (!mLevel->mIsClientSide)
        {
            actuallyHurt(source, damage - mLastHurt);
        }

        mLastHurt = damage;
        mHurtDir = 0.0f;
        freshHurt = false;
    }
    else
    {
        mLastHurt = damage;
        mInvulnerableTime = mInvulnerableDuration;

        if (!mLevel->mIsClientSide)
        {
            actuallyHurt(source, damage);
        }

        mHurtDuration = 10;
        mHurtTime = 10;
        mHurtDir = 0.0f;
    }

    boost::shared_ptr<Entity> attacker = source->getEntity();

    if (attacker)
    {
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, attacker->GetType()))
        {
            mHurtByPlayerOnPeaceful = false;
            setLastHurtByMob(boost::static_pointer_cast<LivingEntity>(attacker));
        }

        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType()))
        {
            if (mLevel->getDifficulty() == Difficulty::PEACEFUL)
            {
                mHurtByPlayerOnPeaceful = true;
            }

            mLastHurtByPlayerTime = 100;
            mLastHurtByPlayer = boost::static_pointer_cast<Player>(attacker);
        }
        else if (eTYPE_DERIVED_FROM(EntityType::WOLF, attacker->GetType()))
        {
            boost::shared_ptr<Wolf> wolf = boost::static_pointer_cast<Wolf>(attacker);

            if (wolf->isTame())
            {
                mLastHurtByPlayerTime = 100;
                mLastHurtByPlayer.reset();
            }
        }
    }

    if (source->isFireProjectile())
    {
        setSecondsOnFire(5);
    }

    if (mLevel->mIsClientSide)
    {
        return false;
    }

    if (freshHurt)
    {
        if (blocked)
        {
            mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::ATTACK_BLOCKED, 0);
        }
        else if (dynamic_cast<const EntityDamageSource*>(source) != NULL
                 && static_cast<const EntityDamageSource*>(source)->isThorns())
        {
            mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::THORNED, 0);
        }
        else
        {
            unsigned char event = 2;

            if (source == DamageSource::DROWN)
            {
                event = 36;
            }
            else if (source->isFire())
            {
                event = 37;
            }

            mLevel->broadcastEntityEvent(shared_from_this(), event, 0);
        }

        if (source != DamageSource::DROWN && (!blocked || damage > 0.0f))
        {
            markHurt();
        }

        if (attacker)
        {
            double dx = attacker->mX - mX;
            double dz = attacker->mZ - mZ;

            while (dx * dx + dz * dz < 0.0001)
            {
                dx = (Math::random() - Math::random()) * 0.01;
                dz = (Math::random() - Math::random()) * 0.01;
            }

            mHurtDir = (float)(atan2(dz, dx) * 180.0 / 3.1415927410125732 - (double)mYRot);
            knockback(attacker, 0.4f, dx, dz);
        }
        else
        {
            mHurtDir = (float)((int)(Math::random() * 2.0) * 180);
        }
    }

    MemSect(31);

    if (getHealth() <= 0.0f)
    {
        if (!checkTotemDeathProtection(source))
        {
            const SoundEvent* deathSound = getDeathSound();

            if (freshHurt && deathSound != NULL)
            {
                playSound(deathSound, getSoundVolume(), getVoicePitch());
            }

            die(source);
        }
    }
    else if (freshHurt)
    {
        const SoundEvent* hurtSound = getHurtSound(source);

        if (hurtSound != NULL)
        {
            playSound(hurtSound, getSoundVolume(), getVoicePitch());
        }
    }

    MemSect(0);

    bool damaged = !blocked || damage > 0.0f;

    if (damaged)
    {
        if (mLastDamageSource != NULL)
        {
            delete mLastDamageSource;
        }

        mLastDamageSource = new DamageSource(*source);
        mLastDamageStamp = mLevel->getGameTime();
    }

    return damaged;
}

const DamageSource* LivingEntity::getLastDamageSource()
{
    if (mLevel->getGameTime() - mLastDamageStamp > 40LL)
    {
        delete mLastDamageSource;
        mLastDamageSource = NULL;
    }

    return mLastDamageSource;
}

void LivingEntity::blockUsingShield(const boost::shared_ptr<LivingEntity>& attacker)
{
    attacker->knockback(shared_from_this(), 0.5f, mX - attacker->mX, mZ - attacker->mZ);
}

void LivingEntity::playHurtSound(const DamageSource* source)
{
    const SoundEvent* sound = getHurtSound(source);

    if (sound != NULL)
    {
        playSound(sound, getSoundVolume(), getVoicePitch());
    }
}

bool LivingEntity::checkTotemDeathProtection(const DamageSource* source)
{
    if (source->isBypassInvul())
    {
        return false;
    }

    not_null_ptr<ItemInstance> totem;
    not_null_ptr<ItemInstance> held = ItemInstance::EMPTY;

    for (int hand = 0; hand < 2; ++hand)
    {
        held = getItemInHand((InteractionHand::EInteractionHand)hand);

        if (held->getItem() == Items::totemOfUndying)
        {
            totem = held->copy();
            held->shrink(1);
            break;
        }
    }

    if (totem)
    {
        setHealth(1.0f);
        removeAllEffects();
        addEffect(new MobEffectInstance(MobEffects::REGENERATION, 800, 1), boost::shared_ptr<Entity>());
        addEffect(new MobEffectInstance(MobEffects::ABSORPTION, 100, 1), boost::shared_ptr<Entity>());
        addEffect(new MobEffectInstance(MobEffects::FIRE_RESISTANCE, 800, 1), boost::shared_ptr<Entity>());
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::TALISMAN_ACTIVATE, 0);
    }

    return totem != NULL;
}

bool LivingEntity::isDamageSourceBlocked(const DamageSource* source)
{
    return !source->isBypassArmor() && isBlocking();
}

void LivingEntity::breakItem(not_null_ptr<ItemInstance> itemInstance)
{
    if (!mLevel->mIsClientSide)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::ITEM_BREAK, getSoundSource(), 0.8f,
                          mLevel->mRandom->nextFloat() * 0.4f + 0.8f, 16.0f);
    }

    for (int i = 0; i < 5; ++i)
    {
        Vec3* velocity = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0.0);
        velocity = velocity->xRot(-(mXRot * Mth::DEG_TO_RAD));
        velocity = velocity->yRot(-(mYRot * Mth::DEG_TO_RAD));

        Vec3* offset = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.3, -mRandom->nextFloat() * 0.6 - 0.3, 0.6);
        offset = offset->xRot(-(mXRot * Mth::DEG_TO_RAD));
        offset = offset->yRot(-(mYRot * Mth::DEG_TO_RAD));
        offset = offset->add(mX, mY + (double)getEyeHeight(), mZ);

        int particleData[1] = {Item::VALIDgetId(itemInstance->getItem())};
        mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z, velocity->x, velocity->y + 0.05,
                            velocity->z, arrayWithLength<int>(particleData, 1));
    }
}

void LivingEntity::die(const DamageSource* source)
{
    if (mDead)
    {
        return;
    }

    boost::shared_ptr<Entity> killer = source->getEntity();
    boost::shared_ptr<LivingEntity> credit = getKillCredit();

    if (mDeathScore >= 0 && credit)
    {
        credit->awardKillScore(shared_from_this(), mDeathScore, source);
    }

    if (killer)
    {
        killer->killed(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
    }

    mDead = true;
    getCombatTracker()->recheckStatus();

    if (!mLevel->mIsClientSide)
    {
        int looting = 0;
        boost::shared_ptr<Player> player;

        if (killer && eTYPE_DERIVED_FROM(EntityType::PLAYER, killer->GetType()))
        {
            player = boost::dynamic_pointer_cast<Player>(killer);
            looting = EnchantmentHelper::getMobLooting(boost::dynamic_pointer_cast<LivingEntity>(player));
        }

        if (shouldDropExperience() && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT))
        {
            dropAllDeathLoot(mLastHurtByPlayerTime > 0, looting, source);
        }

        if (player)
        {
            player->awardStat(
                gStats->get_killMob(),
                GenericStats::param_mobKill(player, boost::dynamic_pointer_cast<Mob>(shared_from_this()), source));
        }
    }

    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::DEATH, 0);
}

void LivingEntity::dropEquipment(bool hitByPlayer, int lootingLevel)
{
}

void LivingEntity::knockback(boost::shared_ptr<Entity> attacker, float strength, double x, double z)
{
    AttributeInstance* resistance = getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE);

    if (mRandom->nextDouble() < resistance->getValue())
    {
        return;
    }

    mHasImpulse = true;

    double horizontal = Mth::sqrt(x * x + z * z);

    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureReducedKnockback())
    {
        strength = 0.1f;
    }

    mXd *= 0.5;
    mYd *= 0.5;
    mZd *= 0.5;
    mXd -= x / horizontal * strength;
    mZd -= z / horizontal * strength;

    if (mOnGround)
    {
        mYd *= 0.5;
        mYd += strength;

        if (mYd > 0.4f)
        {
            mYd = 0.4f;
        }
    }
}

const SoundEvent* LivingEntity::getHurtSound(const DamageSource* source)
{
    return SoundEvents::GENERIC_HURT;
}

const SoundEvent* LivingEntity::getDeathSound()
{
    return SoundEvents::GENERIC_DEATH;
}

const SoundEvent* LivingEntity::getFallDamageSound(int distance)
{
    if (distance > 4)
    {
        return SoundEvents::GENERIC_BIG_FALL;
    }

    return SoundEvents::GENERIC_SMALL_FALL;
}

void LivingEntity::dropAllDeathLoot(bool hitByPlayer, int lootingLevel, const DamageSource* source)
{
    dropDeathLoot(mLastHurtByPlayerTime > 0, lootingLevel);
    dropEquipment(mLastHurtByPlayerTime > 0, lootingLevel);
}

void LivingEntity::dropDeathLoot(bool hitByPlayer, int lootingLevel)
{
}

bool LivingEntity::trapdoorUsableAsLadder(const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(TrapDoorBlock::OPEN))
    {
        return false;
    }

    const BlockState* below = mLevel->getBlockState(mTickCache, pos.below());

    if (below->getBlock() != Blocks::ladder)
    {
        return false;
    }

    return below->getValue(LadderBlock::FACING) == state->getValue(TrapDoorBlock::FACING);
}

bool LivingEntity::onLadder()
{
    int x = Mth::floor(mX);
    int y = Mth::floor(mBoundingBox->mMinY);
    int z = Mth::floor(mZ);

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()) && static_cast<Player*>(this)->isSpectator())
    {
        return false;
    }

    BlockPos pos(x, y, z);
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);
    Block* block = state->getBlock();

    if (block != Blocks::ladder && block != Blocks::vine)
    {
        return typeid(*block) == typeid(TrapDoorBlock) && trapdoorUsableAsLadder(pos, state);
    }

    return true;
}

bool LivingEntity::isShootable()
{
    return true;
}

bool LivingEntity::isAlive()
{
    return !mRemoved && getHealth() > 0.0f;
}

void LivingEntity::causeFallDamage(float fallDistance, float damageMultiplier)
{
    Entity::causeFallDamage(fallDistance, damageMultiplier);

    MobEffectInstance* jumpBoost = getEffect(MobEffects::JUMP);
    float jumpReduction = jumpBoost == NULL ? 0.0f : jumpBoost->getAmplifier() + 1.0f;
    int damage = (int)ceilf((fallDistance - 3.0f - jumpReduction) * damageMultiplier);

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && !TakeGlideCollisionDamage())
    {
        return;
    }

    if (damage < 1)
    {
        return;
    }

    playSound(getFallDamageSound(damage), 1.0f, 1.0f);
    hurt(DamageSource::FALL, (float)damage);

    BlockPos pos(Mth::floor(mX), Mth::floor(mY - 0.20000000298023224), Mth::floor(mZ));
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);

    if (state->getMaterial() != Material::AIR)
    {
        const SoundType* soundType = state->getBlock()->getSoundType();

        MemSect(31);
        playSound(soundType->getFallSound(), soundType->getVolume() * 0.5f, soundType->getPitch() * 0.75f);
        MemSect(0);
    }
}

void LivingEntity::animateHurt()
{
    mHurtDuration = 10;
    mHurtTime = 10;
    mHurtDir = 0.0f;
}

int LivingEntity::getArmorValue()
{
    return Mth::floor(getAttribute(SharedMonsterAttributes::ARMOR)->getValue());
}

void LivingEntity::hurtArmor(float damage)
{
}

void LivingEntity::hurtCurrentlyUsedShield(float damage)
{
}

float LivingEntity::getDamageAfterArmorAbsorb(const DamageSource* source, float damage)
{
    if (source->isBypassArmor())
    {
        return damage;
    }

    hurtArmor(damage);

    return CombatRules::getDamageAfterAbsorb(damage, (float)getArmorValue(),
                                             (float)getAttribute(SharedMonsterAttributes::ARMOR_TOUGHNESS)->getValue());
}

float LivingEntity::getDamageAfterMagicAbsorb(const DamageSource* source, float damage)
{
    if (source->isBypassMagic())
    {
        return damage;
    }

    if (hasEffect(MobEffects::DAMAGE_RESISTANCE) && source != DamageSource::OUT_OF_WORLD
        && source != DamageSource::CREATIVE_PLAYER)
    {
        int amplifier = getEffect(MobEffects::DAMAGE_RESISTANCE)->getAmplifier();
        damage = damage * (float)(25 - (amplifier * 5 + 5)) / 25.0f;
    }

    if (damage <= 0.0f)
    {
        return 0.0f;
    }

    NonNullList<not_null_ptr<ItemInstance> > armorSlots = getArmorSlots();
    int protection = EnchantmentHelper::getDamageProtection(armorSlots, source);

    if (protection > 0)
    {
        damage = CombatRules::getDamageAfterMagicAbsorb(damage, (float)protection);
    }

    return damage;
}

void LivingEntity::actuallyHurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source) && !source->isBypassInvul())
    {
        return;
    }

    damage = getDamageAfterArmorAbsorb(source, damage);
    damage = getDamageAfterMagicAbsorb(source, damage);

    float absorbed = damage - getAbsorptionAmount();

    if (absorbed < 0.0f)
    {
        absorbed = 0.0f;
    }

    setAbsorptionAmount(getAbsorptionAmount() - (damage - absorbed));

    if (absorbed != 0.0f)
    {
        float health = getHealth();
        setHealth(health - absorbed);
        getCombatTracker()->recordDamage(source, health, absorbed);
        setAbsorptionAmount(getAbsorptionAmount() - absorbed);
    }
}

void LivingEntity::spawnEatParticles(const ItemInstance& itemInstance, int count)
{
    for (int i = 0; i < count; ++i)
    {
        Vec3* velocity = Vec3::newTemp((mRandom->nextDouble() - 0.5) * 0.1, mRandom->nextDouble() * 0.1 + 0.1, 0.0);
        velocity = velocity->xRot(-(mXRot * Mth::DEG_TO_RAD));
        velocity = velocity->yRot(-(mYBodyRot * Mth::DEG_TO_RAD));
        velocity->y = velocity->y + 0.05;

        double offsetX = (mRandom->nextDouble() - 0.5) * 0.3;
        double offsetY = -mRandom->nextDouble() * 0.6 - 0.3;
        Vec3* offset = Vec3::newTemp(offsetX, offsetY, 0.6);
        offset = offset->xRot(-(mXRot * Mth::DEG_TO_RAD));
        offset = offset->yRot(-(mYBodyRot * Mth::DEG_TO_RAD));
        offset = offset->add(mX, mY + (double)getEyeHeight(), mZ);

        int itemId = Item::getId(itemInstance.getItem());
        arrayWithLength<int> particleData(&itemId, 1);

        mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z, velocity->x, velocity->y, velocity->z,
                            particleData);
    }
}

CombatTracker* LivingEntity::getCombatTracker()
{
    return mCombatTracker;
}

boost::shared_ptr<LivingEntity> LivingEntity::getKillCredit()
{
    if (mCombatTracker->getKiller())
    {
        return mCombatTracker->getKiller();
    }

    if (mLastHurtByPlayer)
    {
        return mLastHurtByPlayer;
    }

    boost::shared_ptr<LivingEntity> attacker = mLastHurtByMob.lock();
    return attacker;
}

float LivingEntity::getMaxHealth()
{
    return (float)getAttribute(SharedMonsterAttributes::MAX_HEALTH)->getValue();
}

int LivingEntity::getArrowCount()
{
    return mEntityData->get(DATA_ARROW_COUNT_ID);
}

void LivingEntity::setArrowCount(int arrowCount)
{
    mEntityData->set(DATA_ARROW_COUNT_ID, arrowCount, true);
}

int LivingEntity::getCurrentSwingDuration()
{
    int amplification = 0;

    if (hasEffect(MobEffects::DIG_SPEED))
    {
        amplification = getEffect(MobEffects::DIG_SPEED)->getAmplifier();
    }

    if (hasEffect(MobEffects::CONDUIT_POWER))
    {
        int conduit = getEffect(MobEffects::CONDUIT_POWER)->getAmplifier() + 1;

        if (amplification < conduit)
        {
            amplification = conduit;
        }
    }

    if (amplification > 0)
    {
        return 5 - amplification;
    }

    if (hasEffect(MobEffects::DIG_SLOWDOWN))
    {
        return getEffect(MobEffects::DIG_SLOWDOWN)->getAmplifier() * 2 + 8;
    }

    return 6;
}

bool LivingEntity::swing(InteractionHand::EInteractionHand hand)
{
    int swingTime = mSwingTime;
    bool wasSwinging = mSwinging;

    if (wasSwinging && swingTime >= 0 && swingTime < getCurrentSwingDuration() / 2)
    {
        return false;
    }

    mSwingTime = -1;

    if (wasSwinging)
    {
        if (swingTime != -1)
        {
            ++mSwingCount;
        }
    }
    else
    {
        mSwingCount = 0;
    }

    mSwingingArm = hand;
    mSwinging = true;

    ServerLevel* serverLevel = dynamic_cast<ServerLevel*>(mLevel);

    if (serverLevel != NULL)
    {
        serverLevel->getTracker()->broadcast(
            shared_from_this(), boost::shared_ptr<Packet>(new ClientboundAnimatePacket(
                                    shared_from_this(), hand == InteractionHand::eInteractionHand_MainHand ? 0 : 3)));
    }

    return true;
}

void LivingEntity::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::DEATH)
    {
        MemSect(31);

        const SoundEvent* deathSound = getDeathSound();

        if (deathSound != NULL)
        {
            playSound(deathSound, getSoundVolume(), (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
        }

        MemSect(0);

        setHealth(0.0f);
        die(DamageSource::GENERIC);
        return;
    }

    if (event == EntityEvent::SHIELD_DISABLED)
    {
        playSound(SoundEvents::SHIELD_BREAK, 0.8f,
                  (float)(mLevel->mRandom->nextFloat() * 0.4000000059604645 + 0.800000011920929));
        return;
    }

    if (event != EntityEvent::HURT && event != EntityEvent::ATTACK_BLOCKED && event != EntityEvent::THORNED
        && event != EntityEvent::DROWNED && event != EntityEvent::BURNED)
    {
        Entity::handleEntityEvent(event, data);
        return;
    }

    mHurtTime = 10;
    mAnimationSpeed = 1.5f;
    mHurtDir = 0.0f;
    mInvulnerableTime = mInvulnerableDuration;
    mHurtDuration = 10;

    if (event == EntityEvent::THORNED)
    {
        playSound(SoundEvents::ENCHANT_THORNS_HIT, getSoundVolume(),
                  (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
    }
    else if (event == EntityEvent::ATTACK_BLOCKED)
    {
        playSound(SoundEvents::SHIELD_BLOCK, 1.0f,
                  (float)(mLevel->mRandom->nextFloat() * 0.4000000059604645 + 0.800000011920929));
    }

    const DamageSource* source = DamageSource::GENERIC;

    if (event == EntityEvent::BURNED)
    {
        source = DamageSource::ON_FIRE;
    }
    else if (event == EntityEvent::DROWNED)
    {
        source = DamageSource::DROWN;
    }

    MemSect(31);

    const SoundEvent* hurtSound = getHurtSound(source);

    if (hurtSound != NULL)
    {
        playSound(hurtSound, getSoundVolume(), (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
    }

    MemSect(0);

    hurt(DamageSource::GENERIC, 0.0f);
}

void LivingEntity::outOfWorld()
{
    hurt(DamageSource::OUT_OF_WORLD, 4.0f);
}

void LivingEntity::updateSwingTime()
{
    int duration = getCurrentSwingDuration();

    if (mSwinging)
    {
        int swingTime = mSwingTime + 1;

        if (swingTime < duration)
        {
            mSwingTime = swingTime;
        }
        else
        {
            mSwingTime = 0;
            mSwinging = false;
        }
    }
    else
    {
        mSwingTime = 0;
    }

    mAttackAnim = (float)mSwingTime / (float)duration;
}

AttributeInstance* LivingEntity::getAttribute(Attribute* attribute)
{
    return getAttributes()->getInstance(attribute);
}

BaseAttributeMap* LivingEntity::getAttributes()
{
    if (mAttributes == NULL)
    {
        mAttributes = new ModifiableAttributeMap();
    }

    return mAttributes;
}

EMobType LivingEntity::getMobType()
{
    return MOB_TYPE_UNDEFINED;
}

not_null_ptr<ItemInstance> LivingEntity::getMainHandItem()
{
    return getItemSlot(EquipmentSlot::MAINHAND);
}

not_null_ptr<ItemInstance> LivingEntity::getOffhandItem()
{
    return getItemSlot(EquipmentSlot::OFFHAND);
}

not_null_ptr<ItemInstance> LivingEntity::getItemInHand(InteractionHand::EInteractionHand hand)
{
    if (hand == InteractionHand::eInteractionHand_MainHand)
    {
        return getItemSlot(EquipmentSlot::MAINHAND);
    }

    if (hand == InteractionHand::eInteractionHand_OffHand)
    {
        return getItemSlot(EquipmentSlot::OFFHAND);
    }

    return ItemInstance::EMPTY;
}

void LivingEntity::setItemInHand(InteractionHand::EInteractionHand hand, not_null_ptr<ItemInstance> itemInstance)
{
    if (hand == InteractionHand::eInteractionHand_MainHand)
    {
        setItemSlot(EquipmentSlot::MAINHAND, itemInstance);
    }
    else if (hand == InteractionHand::eInteractionHand_OffHand)
    {
        setItemSlot(EquipmentSlot::OFFHAND, itemInstance);
    }
}

bool LivingEntity::hasItemInSlot(const EquipmentSlot* slot)
{
    return !getItemSlot(slot)->isEmpty();
}

void LivingEntity::setSprinting(bool sprinting)
{
    Entity::setSprinting(sprinting);

    AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);

    if (speed->getModifier(eMODIFIER_ID_SprintingSpeedBoost) != NULL)
    {
        speed->removeModifier(eMODIFIER_ID_SprintingSpeedBoost);
    }

    if (sprinting)
    {
        speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_SPRINTING));
    }
}

TextureAtlasSprite* LivingEntity::getItemInHandIcon(not_null_ptr<ItemInstance> itemInstance, int frame)
{
    Item* item = itemInstance->getItem();
    return item->getLayerIcon(itemInstance->getAuxValue(), frame, itemInstance);
}

float LivingEntity::getSoundVolume()
{
    return 1.0f;
}

float LivingEntity::getVoicePitch()
{
    if (isBaby())
    {
        return (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.5f;
    }

    return (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f;
}

bool LivingEntity::isImmobile()
{
    return getHealth() <= 0.0f;
}


void LivingEntity::findStandUpPosition(boost::shared_ptr<Entity> vehicle)
{
    if (eTYPE_DERIVED_FROM(EntityType::BOAT, vehicle->GetType())
        || eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, vehicle->GetType()))
    {
        double distance = (double)(mBbWidth * 0.5f + vehicle->mBbWidth * 0.5f) + 0.4;
        float angle;

        if (eTYPE_DERIVED_FROM(EntityType::BOAT, vehicle->GetType()))
        {
            angle = 0.0f;
        }
        else
        {
            angle = 1.5707964f * (getMainArm() == HumanoidArm::RIGHT ? -1.0f : 1.0f);
        }

        double dirX = -(double)Mth::sin(-(mYRot * Mth::DEG_TO_RAD) - 3.1415927f + angle);
        double dirZ = -(double)Mth::cos(-(mYRot * Mth::DEG_TO_RAD) - 3.1415927f + angle);
        double scale = fabs(dirX) - fabs(dirZ) < 0.0 ? fabs(dirZ) : fabs(dirX);

        distance = distance / scale;

        double newX = mX + dirX * distance;
        double newZ = mZ + dirZ * distance;

        setPos(newX, vehicle->mY + (double)vehicle->mBbHeight + 0.001, newZ);

        if (mLevel->hasBlockCubes(mBoundingBox))
        {
            setPos(newX, vehicle->mY + (double)vehicle->mBbHeight + 1.001, newZ);

            if (mLevel->hasBlockCubes(mBoundingBox))
            {
                setPos(vehicle->mX, vehicle->mY + (double)mBbHeight + 0.001, vehicle->mZ);
            }
        }
    }
    else
    {
        double standX = vehicle->mX;
        double standY = vehicle->mBoundingBox->mMinY + (double)vehicle->mBbHeight;
        double standZ = vehicle->mZ;

        const Direction* forward = vehicle->getMotionDirection();

        if (forward != NULL)
        {
            const Direction* side = forward->getClockWise();
            const int offsets[9][2] = {{0, 1}, {0, -1}, {-1, 1}, {-1, -1}, {1, 1}, {1, -1}, {-1, 0}, {1, 0}, {0, 1}};

            double baseX = floor(mX) + 0.5;
            double baseZ = floor(mZ) + 0.5;

            double width = mBoundingBox->mMaxX - mBoundingBox->mMinX;
            double depth = mBoundingBox->mMaxZ - mBoundingBox->mMinZ;

            AABB* box = AABB::newPermanent(baseX - width * 0.5, vehicle->mBoundingBox->mMinY, baseZ - depth * 0.5,
                                           baseX + width * 0.5, floor(vehicle->mBoundingBox->mMinY) + (double)mBbHeight,
                                           baseZ + depth * 0.5);

            for (int i = 0; i < 9; ++i)
            {
                double dx = (double)(forward->getStepX() * offsets[i][0] + side->getStepX() * offsets[i][1]);
                double dz = (double)(forward->getStepZ() * offsets[i][0] + side->getStepZ() * offsets[i][1]);

                if (dx == 0.0 && dz == 0.0)
                {
                    continue;
                }

                int blockX = (int)(mX + dx);
                int blockZ = (int)(mZ + dz);

                if (mLevel->hasBlockCubes(box->move(dx, 0.0, dz)))
                {
                    continue;
                }

                BlockPos here(blockX, (int)mY, blockZ);

                if (mLevel->getBlockState(mTickCache, here)->isTopSolidBlocking())
                {
                    teleportTo(mX + dx, floor(mY) + 1.0, mZ + dz);
                    return;
                }

                if (mLevel->getBlockState(mTickCache, BlockPos(blockX, (int)mY - 1, blockZ))->isTopSolidBlocking()
                    || mLevel->getBlockState(mTickCache, BlockPos(blockX, (int)mY - 1, blockZ))->getMaterial()
                           == Material::WATER)
                {
                    standX = mX + dx;
                    standY = floor(mY) + 1.0;
                    standZ = mZ + dz;
                }
            }
        }

        teleportTo(standX, standY, standZ);
    }
}

bool LivingEntity::shouldShowName()
{
    return isCustomNameVisible();
}

float LivingEntity::getJumpPower()
{
    return 0.42f;
}

void LivingEntity::jumpFromGround()
{
    mYd = (double)getJumpPower();

    if (hasEffect(MobEffects::JUMP))
    {
        mYd = mYd + (double)((float)(getEffect(MobEffects::JUMP)->getAmplifier() + 1) * 0.1f);
    }

    if (isSprinting())
    {
        float radians = mYRot * Mth::DEG_TO_RAD;
        mXd = mXd - (double)(Mth::sin(radians) * 0.2f);
        mZd = mZd + (double)(Mth::cos(radians) * 0.2f);
    }

    mHasImpulse = true;
}

void LivingEntity::jumpInWater()
{
    mYd = mYd + 0.07999999821186066;
}

void LivingEntity::goDownInWater()
{
    mYd = mYd - 0.03999999910593033;
}

void LivingEntity::jumpInLava()
{
    mYd = mYd + 0.03999999910593033;
}

float LivingEntity::getWaterSlowDown()
{
    return 0.8f;
}

float LivingEntity::getFrictionForTravel()
{
    MutableBlockPos pos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY - 1.0), Mth::floor(mZ));

    float friction = 0.91f;

    if (!mOnGround)
    {
        return friction;
    }

    Block* block = mLevel->getBlockState(mTickCache, pos)->getBlock();

    if (IsCreativeFlying()
        && (block == Blocks::ice || block == Blocks::packed_ice || block == Blocks::frosted_ice
            || block == Blocks::blue_ice || block == Blocks::slime))
    {
        return Blocks::stone->mFriction * friction;
    }

    return mLevel->getBlockState(mTickCache, pos)->getBlock()->mFriction * friction;
}

bool LivingEntity::shouldApplyWaterGravity()
{
    if ((GetType() & EntityType::WATER_ANIMAL) != EntityType::WATER_ANIMAL && GetType() != EntityType::DOLPHIN
        && GetType() != EntityType::TURTLE)
    {
        return true;
    }

    return !isImmersedInWater();
}

void LivingEntity::activateElytraSpeedBoost(double boost)
{
    mElytraSpeedBoost = boost;
    mElytraSpeedBoostActive = true;
}

void LivingEntity::CheckThermalAreas()
{
    SetLiftForceModifier(1.0);

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();

    if (options == NULL)
    {
        return;
    }

    LevelRuleset* ruleset = options->getRequiredGameRules();

    const bool isGlide = gApp.GetFirstMiniGameType() == eMiniGameId_Glide;

    if (!isGlide || ruleset == NULL)
    {
        return;
    }

    int boostMod = 0;
    bool inside = false;
    bool boostModsSet = false;
    int boostDirection = 0;

    std::vector<ThermalAreaRuleDefinition*> areas;
    ruleset->getThermalAreas(&areas);

    for (std::vector<ThermalAreaRuleDefinition*>::iterator it = areas.begin(); it != areas.end(); ++it)
    {
        if ((*it)->isEnabled() && (*it)->getConditionsMet(shared_from_this())
            && (*it)->getArea()->intersects(getSweptVolume()))
        {
            inside = true;

            if (!mThermalAreasChecked)
            {
                if (mThermalAreaChanged)
                {
                    mThermalAreaChanged = false;

                    if ((*it)->getElytraSpeedBoost() == 0.0)
                    {
                        if (mLevel->mIsClientSide)
                        {
                            if (eTYPE_DERIVED_FROM(EntityType::LOCAL_PLAYER, GetType()))
                            {
                                Minecraft::GetInstance()->mSoundEngine->playUI(SoundEvents::MG03_THERMAL_BOOST, 1.0f,
                                                                               1.0f);
                            }
                        }

                        if (!mLevel->mIsClientSide)
                        {
                            if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, GetType()))
                            {
                                boost::shared_ptr<Packet> packet(new ClientboundSoundPacket(
                                    ClientboundSoundPacket::eSoundInstances_FollowEntity,
                                    SoundEvents::MG03_THERMAL_BOOST, 1.0f, 1.0f, false, getId()));

                                PlayerList* players = (gMinecraftServer != NULL) ? gMinecraftServer->mPlayers : NULL;
                                players->broadcast(boost::static_pointer_cast<ServerPlayer>(shared_from_this()), mX, mY,
                                                   mZ, 30.0, mDimension, packet);
                            }
                        }
                    }

                    if ((*it)->getElytraSpeedBoost() != 0.0)
                    {
                        if (mLevel->mIsClientSide)
                        {
                            if (eTYPE_DERIVED_FROM(EntityType::LOCAL_PLAYER, GetType()))
                            {
                                Minecraft::GetInstance()->mSoundEngine->playUI(SoundEvents::MG03_BOOST, 1.0f, 1.0f);
                            }
                        }

                        if (!mLevel->mIsClientSide)
                        {
                            if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, GetType()))
                            {
                                boost::shared_ptr<Packet> packet(
                                    new ClientboundSoundPacket(ClientboundSoundPacket::eSoundInstances_FollowEntity,
                                                               SoundEvents::MG03_BOOST, 1.0f, 1.0f, false, getId()));

                                PlayerList* players = (gMinecraftServer != NULL) ? gMinecraftServer->mPlayers : NULL;
                                players->broadcast(boost::static_pointer_cast<ServerPlayer>(shared_from_this()), mX, mY,
                                                   mZ, 30.0, mDimension, packet);
                            }
                        }
                    }
                }

                SetLiftForceModifier((*it)->getLiftForceModifier());

                if (!mInThermalArea)
                {
                    if ((*it)->getLiftSpeed() != 0.0)
                    {
                        mThermalLerpTicks = 0;
                        mThermalLerping = true;
                    }

                    mThermalTargetLiftSpeed = (*it)->getLiftSpeed();
                }

                if ((*it)->getMaxHeight() > 0.0)
                {
                    mInThermalArea = true;
                    mThermalCeiling = (*it)->getMaxHeight();
                }

                mThermalLiftRising = (*it)->getLiftSpeed() >= 0.0;

                if ((*it)->getElytraSpeedBoost() > 0.0)
                {
                    activateElytraSpeedBoost((*it)->getElytraSpeedBoost());
                    boostModsSet = (*it)->setBoostMods(boostMod, boostDirection);
                }

                if (mThermalArea != *it && eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, GetType()))
                {
                    boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(
                        shared_from_this());
                    player->GetGameMode()->RecordThermalEntered();
                }

                mThermalArea = *it;
                break;
            }
        }
    }

    if (mThermalAreasChecked)
    {
        mThermalAreasChecked = inside;
        return;
    }

    if (!inside && mThermalArea != NULL)
    {
        mThermalAreaChanged = true;

        if (!mLevel->mIsClientSide)
        {
            mThermalArea->updateUseCount();
            mThermalArea = NULL;
        }
    }

    if (mInThermalArea)
    {
        if (mY >= mThermalCeiling || mVerticalCollision)
        {
            mInThermalArea = false;
            mThermalLerping = false;
        }

        mThermalLerping = mThermalLerping || mInThermalArea;
    }

    if (mThermalLerping)
    {
        if (mThermalLiftSpeed == 0.0 && mYd != 0.0)
        {
            mThermalLiftSpeed = mYd;
        }

        if (mThermalLiftRising)
        {
            if (mThermalLiftSpeed < mThermalTargetLiftSpeed)
            {
                mThermalLiftSpeed += ((mThermalTargetLiftSpeed - mThermalLiftSpeed) / mThermalTargetLiftSpeed) * 0.1;
            }
            else
            {
                mThermalLiftSpeed = mThermalTargetLiftSpeed;
            }
        }
        else if (mThermalLiftSpeed > mThermalTargetLiftSpeed)
        {
            mThermalLiftSpeed -= ((mThermalTargetLiftSpeed - mThermalLiftSpeed) / mThermalTargetLiftSpeed) * 0.1;
        }
        else
        {
            mThermalLiftSpeed = mThermalTargetLiftSpeed;
        }

        if (!mInThermalArea)
        {
            if (++mThermalLerpTicks > 20)
            {
                mThermalLerping = false;
                mThermalLerpTicks = 0;
            }
        }
    }
    else if (mThermalLiftRising)
    {
        if (mThermalLiftSpeed > 0.0)
        {
            mThermalLiftSpeed -= 0.025f;
        }
        else
        {
            mThermalLiftSpeed = 0.0;
        }
    }
    else if (mThermalLiftSpeed < 0.0)
    {
        mThermalLiftSpeed += 0.025f;
    }
    else
    {
        mThermalLiftSpeed = 0.0;
    }

    if (mElytraSpeedBoostActive)
    {
        if (Mth::sqrt(mXd * mXd + mZd * mZd) < mElytraSpeedBoost)
        {
            if (boostModsSet)
            {
                mXd += Mth::abs(mXd) * 0.1 * boostMod;
                mZd += Mth::abs(mZd) * 0.1 * boostDirection;

                if (mXd * boostMod < 0.0 || mZd * boostDirection < 0.0)
                {
                    mElytraSpeedBoostActive = false;
                }
            }
            else
            {
                mXd *= 1.1;
                mZd *= 1.1;
            }
        }
        else
        {
            mElytraSpeedBoostActive = false;
        }
    }
}

void LivingEntity::fallFlyingTravel(double& xd, double& yd, double& zd, Vec3* lookAngle, float& xRot, float& liftFactor,
                                    double& horizontalSpeed, double lookHorizontal)
{
    if (yd > -0.5)
    {
        liftFactor = 1.0f;
    }

    float pitch = xRot * Mth::DEG_TO_RAD;
    double lookLength = sqrt(lookAngle->x * lookAngle->x + lookAngle->z * lookAngle->z);
    horizontalSpeed = sqrt(xd * xd + zd * zd);

    double length = (double)lookAngle->length();
    float cosPitch = cosf(pitch);
    double ratio = length / 0.4;
    double clamped = ratio < 1.0 ? ratio : 1.0;

    float lift = cosPitch * (cosPitch * (float)clamped) * (float)lookHorizontal;

    double newYd = yd + (lift * 0.06 + -0.08);

    if (newYd < 0.0 && lookLength > 0.0)
    {
        double redirect = newYd * -0.1 * lift;
        yd = newYd + redirect;
        xd = xd + (lookAngle->x * redirect) / lookLength;
        zd = zd + (lookAngle->z * redirect) / lookLength;
    }
    else
    {
        yd = newYd;
    }

    if (lookLength > 0.0)
    {
        if (pitch < 0.0f)
        {
            double dive = horizontalSpeed * -(double)sinf(pitch) * 0.04;
            yd = yd + dive * 3.2;
            xd = xd - (lookAngle->x * dive) / lookLength;
            zd = zd - (lookAngle->z * dive) / lookLength;
        }

        xd = xd + ((lookAngle->x / lookLength) * horizontalSpeed - xd) * 0.1;
        zd = zd + ((lookAngle->z / lookLength) * horizontalSpeed - zd) * 0.1;
    }

    xd = xd * 0.9900000095367432;
    yd = yd * 0.9800000190734863;
    zd = zd * 0.9900000095367432;
}

void LivingEntity::travel(float xxa, float yya, float zza)
{
    Player* player = NULL;
    bool skeletonHorseWithPlayerPassenger = false;

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()))
    {
        player = static_cast<Player*>(this);
    }

    bool isSkeletonHorse = eTYPE_DERIVED_FROM(EntityType::SKELETON_HORSE, GetType());

    if (isSkeletonHorse && hasPassenger(EntityType::PLAYER))
    {
        skeletonHorseWithPlayerPassenger = true;
    }

    if (PositionLocked())
    {
        mXd = 0.0;
        mZd = 0.0;

        if (Minecraft::InMiniGame(eMiniGameId_Tumble, false) || Minecraft::InMiniGame(eMiniGameId_Glide, false))
        {
            mYd = 0.0;
        }
    }

    if (isEffectiveAi() || isControlledByLocalInstance())
    {
        if (isInWater() && !(player != NULL && player->mAbilities.mFlying))
        {
            PIXBeginNamedEvent(0.0f, "Travel in water");

            double oldY = mY;
            float waterSlowdown = isSprinting() ? 0.9f : getWaterSlowDown();
            float moveSpeed = 0.02f;

            if (eTYPE_DERIVED_FROM(EntityType::DOLPHIN, GetType()))
            {
                moveSpeed = getSpeed();
            }

            if (skeletonHorseWithPlayerPassenger)
            {
                float friction = getFrictionForTravel();
                float acceleration = 0.16277136f / (friction * friction * friction);

                if (mOnGround)
                {
                    moveSpeed = getSpeed() * acceleration;
                }
            }

            float depthStrider = (float)EnchantmentHelper::getDepthStrider(
                boost::static_pointer_cast<LivingEntity>(shared_from_this()));

            if (depthStrider > 3.0f)
            {
                depthStrider = 3.0f;
            }

            if (depthStrider > 0.0f)
            {
                waterSlowdown = waterSlowdown + (0.54600006f - waterSlowdown) * depthStrider / 3.0f;
                moveSpeed = moveSpeed + (getSpeed() * 1.0f - moveSpeed) * depthStrider / 3.0f;
            }

            if (!isSwimming())
            {
                mWaterTravelSpeedFactor = 1.0f;
                moveRelative(xxa, yya, zza, moveSpeed * mWaterTravelSpeedFactor);
                move(eMoverType_Self, mXd, mYd, mZd, false);
            }
            else if (mWaterTravelCheckTicks - 1 < 1)
            {
                mWaterTravelCheckTicks = 60;
                AABB* searchBox = mBoundingBox->grow(5.0);
                std::vector<boost::shared_ptr<Entity> >* drownedEntities = mLevel->getEntitiesOfClass(typeid(Drowned),
                                                                                                      searchBox);
                mWaterTravelSpeedFactor = drownedEntities->empty() ? 1.0f : 2.0f;
                delete drownedEntities;

                moveRelative(xxa, yya, zza, moveSpeed * mWaterTravelSpeedFactor);
                move(eMoverType_Self, mXd, mYd, mZd, false);
            }
            else
            {
                --mWaterTravelCheckTicks;
                moveRelative(xxa, yya, zza, moveSpeed * mWaterTravelSpeedFactor);
                move(eMoverType_Self, mXd, mYd, mZd, false);
            }

            if (skeletonHorseWithPlayerPassenger)
            {
                waterSlowdown = getFrictionForTravel();
            }

            mXd *= (double)waterSlowdown;
            mYd *= 0.8f;
            mZd *= (double)waterSlowdown;

            if (player == NULL)
            {
                if (shouldApplyWaterGravity())
                {
                    mYd -= 0.02f;
                }
            }
            else if (!isNoGravity() && !isSwimming())
            {
                mYd -= 0.005;
            }

            if (mHorizontalCollision && isFree(mXd, mYd + 0.6f - mY + oldY, mZd))
            {
                mYd = 0.3f;
            }

            PIXEndNamedEvent();
        }
        else if (isInLava() && !(player != NULL && player->mAbilities.mFlying))
        {
            PIXBeginNamedEvent(0.0f, "Travel in lava");

            double oldY = mY;
            moveRelative(xxa, yya, zza, 0.02f);
            move(eMoverType_Self, mXd, mYd, mZd, false);

            mXd *= 0.5;
            mYd *= 0.5;
            mZd *= 0.5;

            if (!isNoGravity())
            {
                mYd -= 0.02;
            }

            if (mHorizontalCollision && isFree(mXd, mYd + 0.6f - mY + oldY, mZd))
            {
                mYd = 0.3f;
            }

            PIXEndNamedEvent();
        }
        else if (isFallFlying() && !PositionLocked() && !gApp.IsAppPaused())
        {
            CheckThermalAreas();

            double glideLift = mGlideDistance + mThermalLiftSpeed;
            double horizontalSpeed = 0.0;
            double liftForce = GetLiftForceModifier();
            fallFlyingTravel(mXd, mYd, mZd, getLookAngle(), mXRot, mFallDistance, horizontalSpeed, liftForce);

            if (gApp.GetFirstMiniGameType() == eMiniGameId_Glide && player != NULL
                && player->CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID_SlowFall) && mYd < 0.0)
            {
                move(eMoverType_Self, mXd, mYd * 0.2, mZd, false);
            }
            else if (glideLift == 0.0)
            {
                move(eMoverType_Self, mXd, mYd, mZd, false);
            }
            else
            {
                move(eMoverType_Self, mXd, glideLift, mZd, false);

                if (mThermalLiftSpeed != 0.0)
                {
                    mYd = 0.0;
                }
            }
        }
        else
        {
            MutableBlockPos groundPos(Mth::floor(mX), Mth::floor(mBoundingBox->mMinY - 1.0), Mth::floor(mZ));

            float friction = getFrictionForTravel();
            float acceleration = 0.16277136f / (friction * friction * friction);

            PIXBeginNamedEvent(0.0f, "Update speed");

            float speed;

            if (!mOnGround)
            {
                if (GetType() == EntityType::WITHER_BOSS)
                {
                    speed = mFlyingSpeed * acceleration;
                }
                else
                {
                    speed = mFlyingSpeed;
                }
            }
            else
            {
                speed = getSpeed() * acceleration;
            }

            PIXEndNamedEvent();

            PIXBeginNamedEvent(0.0f, "moveRelative");
            moveRelative(xxa, yya, zza, speed);
            PIXEndNamedEvent();

            friction = getFrictionForTravel();

            if (onLadder())
            {
                mXd = (double)Mth::clamp((float)mXd, -0.15f, 0.15f);
                mZd = (double)Mth::clamp((float)mZd, -0.15f, 0.15f);
                mFallDistance = 0.0f;

                if (mYd < -0.15)
                {
                    mYd = -0.15;
                }

                if (isSneaking() && eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()) && mYd < 0.0)
                {
                    mYd = 0.0;
                }
            }

            PIXBeginNamedEvent(0.0f, "move");
            move(eMoverType_Self, mXd, mYd, mZd, false);
            PIXEndNamedEvent();

            if (mHorizontalCollision && onLadder())
            {
                mYd = 0.2;
            }

            PIXBeginNamedEvent(0.0f, "Update y delta");

            if (!hasEffect(MobEffects::LEVITATION))
            {
                groundPos.set(Mth::floor(mX), 0, Mth::floor(mZ));

                bool loadedChunk = true;

                if (mLevel->mIsClientSide)
                {
                    if (!CGameNetworkManager::sInstance.IsHost())
                    {
                        loadedChunk = false;
                    }
                }

                bool chunkPopulated = false;

                if (mLevel->hasChunkAt(groundPos))
                {
                    if (mLevel->getChunkAt(groundPos)->isTerrainPopulated())
                    {
                        chunkPopulated = true;
                    }
                }

                bool lockVertical = false;

                if (PositionLocked()
                    && (Minecraft::InMiniGame(eMiniGameId_Tumble, false)
                        || Minecraft::InMiniGame(eMiniGameId_Glide, false)))
                {
                    lockVertical = true;
                }

                if ((!loadedChunk && !chunkPopulated) || lockVertical)
                {
                    mYd = 0.0;
                }
                else if (!isNoGravity())
                {
                    if (mYd >= 0.0)
                    {
                        mYd = mYd + -0.07999999821186066;
                    }
                    else if (hasEffect(MobEffects::SLOW_FALLING))
                    {
                        mYd = mYd + -0.009999999776482582;
                        mFallDistance = 0.0f;
                    }
                    else
                    {
                        mYd = mYd + -0.07999999821186066;
                    }
                }
            }
            else
            {
                mYd = mYd + (0.05 * (double)(getEffect(MobEffects::LEVITATION)->getAmplifier() + 1) - mYd) * 0.2;
            }

            PIXEndNamedEvent();

            mYd = mYd * 0.9800000190734863;
            mXd = mXd * (double)friction;
            mZd = mZd * (double)friction;
        }
    }

    updateModelAnimation();
}

void LivingEntity::updateModelAnimation()
{
    double dx = mX - mXo;
    double dz = mZ - mZo;

    mAnimationSpeedOld = mAnimationSpeed;

    double dy;

    if (eTYPE_DERIVED_FROM(EntityType::FLYING_MOB, GetType()))
    {
        dy = mY - mYo;
    }
    else
    {
        dy = 0.0;
    }

    float target = (float)(Mth::sqrt(dx * dx + dy * dy + dz * dz) * 4.0f);

    if (target > 1.0f)
    {
        target = 1.0f;
    }

    mAnimationSpeed = mAnimationSpeed + (target - mAnimationSpeed) * 0.4f;
    mAnimationPosition = mAnimationPosition + mAnimationSpeed;
}

void LivingEntity::takeFallFlyingCollisionDamage()
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    double xd = mXd;
    double yd = mYd;
    double zd = mZd;
    float xRot = mXRot;
    float liftFactor = mFallDistance;
    double horizontalSpeed = 0.0;

    double lookHorizontal = GetLiftForceModifier();

    fallFlyingTravel(xd, yd, zd, getLookAngle(), xRot, liftFactor, horizontalSpeed, lookHorizontal);

    if (mHorizontalCollision)
    {
        double speed = sqrt(xd * xd + zd * zd);
        float damage = (float)((horizontalSpeed - speed) * 10.0 - 3.0);

        if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        {
            damage = TakeGlideCollisionDamage() ? 1.0f : 0.0f;
        }

        if (hurt(DamageSource::FALL, damage))
        {
            playSound(getFallDamageSound((int)damage), 1.0f, 1.0f);
        }
    }
    else if (mVerticalCollision && Minecraft::InMiniGame(eMiniGameId_Glide, true) && TakeGlideCollisionDamage()
             && hurt(DamageSource::FALL, 1.0f))
    {
        playSound(getFallDamageSound(1), 1.0f, 1.0f);
    }

    if (mOnGround)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && TakeGlideCollisionDamage())
        {
            float health = getHealth();

            if (hurt(DamageSource::FALL, health + 1.0f))
            {
                playSound(getFallDamageSound((int)(health + 1.0f)), 1.0f, 1.0f);
            }
        }

        setSharedFlag(FLAG_FALL_FLYING, false);
    }
}

int LivingEntity::getLightColor()
{
    AABB* box = mBoundingBox;

    double minX = box->mMinX;
    double minY = box->mMinY;
    double minZ = box->mMinZ;
    double maxX = box->mMaxX;
    double maxY = box->mMaxY;
    double maxZ = box->mMaxZ;

    float blockLight = 0.0f;
    float skyLight = 0.0f;
    bool riding = false;

    if (isPassenger())
    {
        if (eTYPE_DERIVED_FROM(EntityType::MINECART, getVehicle()->GetType())
            || eTYPE_DERIVED_FROM(EntityType::BOAT, getVehicle()->GetType()))
        {
            riding = true;
        }
    }

    int minBlockX = mLightSampleMinX;
    int minBlockY = mLightSampleMinY;
    int minBlockZ = mLightSampleMinZ;
    int maxBlockX = mLightSampleMaxX;
    int maxBlockY = mLightSampleMaxY;
    int maxBlockZ = mLightSampleMaxZ;
    float volume;

    if (riding)
    {
        minY = minY + 1.0;

        if (maxY <= minY)
        {
            maxY = minY + 0.1;
        }

        minBlockX = Mth::floor(minX);
        minBlockY = Mth::floor(minY);
        minBlockZ = Mth::floor(minZ);
        maxBlockX = Mth::floor(maxX);
        maxBlockY = Mth::floor(maxY);
        maxBlockZ = Mth::floor(maxZ);
    }

    volume = (float)((maxX - minX) * (maxY - minY) * (maxZ - minZ));

    int sampleCount = ((maxBlockX - minBlockX) + 1) * ((maxBlockY - minBlockY) + 1) * ((maxBlockZ - minBlockZ) + 1);

    int* samples = mLightSamples;

    if (sampleCount > 32)
    {
        samples = new int[sampleCount];
    }

    bool resample = true;

    if (mLightCacheTicks >= 0)
    {
        if (mLightCacheTicks < 1)
        {
            int colour = Entity::getLightColor();

            if (colour != mLastLightColor)
            {
                mLastLightColor = colour;
                ++mLightCacheTicks;
            }

            if (mLightCacheTicks < 1)
            {
                resample = false;
            }
        }

        if (resample)
        {
            --mLightCacheTicks;
        }
    }
    else
    {
        resample = false;
    }

    if (resample || sampleCount > 32)
    {
        int index = 0;

        for (int x = minBlockX; x <= maxBlockX; ++x)
        {
            for (int y = minBlockY; y <= maxBlockY; ++y)
            {
                for (int z = minBlockZ; z <= maxBlockZ; ++z)
                {
                    samples[index] = mLevel->getLightColor(BlockPos(x, y, z), 0, -1);
                    ++index;
                }
            }
        }
    }

    int index = 0;

    for (int x = minBlockX; x <= maxBlockX; ++x)
    {
        float lowX = (float)x;
        float highX = lowX + 1.0f;

        if (lowX < minX)
        {
            lowX = (float)minX;
        }

        if (maxX < highX)
        {
            highX = (float)maxX;
        }

        for (int y = minBlockY; y <= maxBlockY; ++y)
        {
            float lowY = (float)y;
            float highY = lowY + 1.0f;

            if (lowY < minY)
            {
                lowY = (float)minY;
            }

            if (maxY < highY)
            {
                highY = (float)maxY;
            }

            float areaXY = (highX - lowX) * (highY - lowY);

            for (int z = minBlockZ; z <= maxBlockZ; ++z)
            {
                float lowZ = (float)z;
                float highZ = lowZ + 1.0f;

                if (lowZ < minZ)
                {
                    lowZ = (float)minZ;
                }

                if (maxZ < highZ)
                {
                    highZ = (float)maxZ;
                }

                float weight = (areaXY * (highZ - lowZ)) / volume;
                int sample = samples[index];
                ++index;

                blockLight = blockLight + weight * (float)(sample & 0xffff);
                skyLight = skyLight + weight * (float)(sample >> 16);
            }
        }
    }

    if (sampleCount > 32)
    {
        delete samples;
    }

    if (blockLight > 240.0f)
    {
        blockLight = 240.0f;
    }

    if (skyLight > 240.0f)
    {
        skyLight = 240.0f;
    }

    return ((int)skyLight << 16) | (int)blockLight;
}

void LivingEntity::setBoundingBox(AABB* boundingBox)
{
    int minX = Mth::floor(boundingBox->mMinX);
    int minY = Mth::floor(boundingBox->mMinY);
    int minZ = Mth::floor(boundingBox->mMinZ);
    int maxX = Mth::floor(boundingBox->mMaxX);
    int maxY = Mth::floor(boundingBox->mMaxY);
    int maxZ = Mth::floor(boundingBox->mMaxZ);

    bool changed = minX != mLightSampleMinX || maxX != mLightSampleMaxX || minY != mLightSampleMinY
                   || maxY != mLightSampleMaxY || minZ != mLightSampleMinZ || maxZ != mLightSampleMaxZ;

    mLightSampleMinX = minX;
    mLightSampleMaxX = maxX;
    mLightSampleMinY = minY;
    mLightSampleMaxY = maxY;
    mLightSampleMinZ = minZ;
    mLightSampleMaxZ = maxZ;

    if (changed)
    {
        ++mLightCacheTicks;
    }

    Entity::setBoundingBox(boundingBox);
}

float LivingEntity::getSpeed()
{
    return mSpeed;
}

void LivingEntity::setSpeed(float speed)
{
    mSpeed = speed;
}

bool LivingEntity::doHurtTarget(boost::shared_ptr<Entity> target)
{
    setLastHurtMob(target);

    return false;
}

bool LivingEntity::isSleeping()
{
    return false;
}


void LivingEntity::updateEquipmentSlots()
{
    for (int i = 0; i < 6; ++i)
    {
        const EquipmentSlot* slot = EquipmentSlot::SLOTS[i];
        not_null_ptr<ItemInstance> previous;

        if (slot->getType() == EquipmentSlot::HAND)
        {
            previous = mLastHandItems[slot->getIndex()];
        }
        else
        {
            previous = mLastArmorItems[slot->getIndex()];
        }

        not_null_ptr<ItemInstance> current = getItemSlot(slot);

        if (ItemInstance::matches(current, previous, true))
        {
            continue;
        }

        static_cast<ServerLevel*>(mLevel)->getTracker()->broadcast(
            shared_from_this(),
            boost::shared_ptr<Packet>(new ClientboundSetEquippedItemPacket(getId(), slot, current)));

        if (!previous->isEmpty())
        {
            mAttributes->removeAttributeModifiers(previous->getAttributeModifiers(slot));
        }

        if (!current->isEmpty())
        {
            mAttributes->addAttributeModifiers(current->getAttributeModifiers(slot));
        }

        if (slot->getType() == EquipmentSlot::HAND)
        {
            mLastHandItems[slot->getIndex()] = current->isEmpty() ? ItemInstance::EMPTY : current->copy();
        }
        else
        {
            mLastArmorItems[slot->getIndex()] = current->isEmpty() ? ItemInstance::EMPTY : current->copy();
        }
    }
}

void LivingEntity::tick()
{
    PIXBeginNamedEvent(0.0f, "Living entity tick");

    mTickCache.buildCache(mLevel, AABB::newTemp(mBoundingBox));

    Entity::tick();

    updatingUsingItem();

    if (!mLevel->mIsClientSide)
    {
        int arrows = getArrowCount();

        if (arrows > 0)
        {
            if (mRemoveArrowTime < 1)
            {
                mRemoveArrowTime = 600 - arrows * 20;
            }

            --mRemoveArrowTime;

            if (mRemoveArrowTime < 1)
            {
                setArrowCount(arrows - 1);
            }
        }

        updateEquipmentSlots();

        if (mTickCount % 20 == 0)
        {
            PIXBeginNamedEvent(0.0f, "Recheck combat tracker status");
            getCombatTracker()->recheckStatus();
            PIXEndNamedEvent();
        }

        if (!mGlowing)
        {
            bool glowing = hasEffect(MobEffects::GLOWING);

            if (getSharedFlag(FLAG_GLOWING) != glowing)
            {
                setSharedFlag(FLAG_GLOWING, glowing);
            }
        }
    }

    if (mInsomniaComponent != NULL)
    {
        mInsomniaComponent->tick();
    }

    const BlockState* state = mLevel->getBlockState(
        mTickCache, BlockPos(mX + (double)(mBbWidth * 0.5f), mY, mZ + (double)(mBbWidth * 0.5f)));

    if (Block::getId(state->getBlock()) == BlockId::BUBBLE_COLUMN_SEGMENT)
    {
        if (!mSwimSoundPlayed)
        {
            if (state->getValue(BubbleColumnSegment::DRAG))
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::BUBBLE_COLUMN_WHIRLPOOL_INSIDE,
                                       SoundSource::eSoundSource_Players, 0.9f, 1.0f, false, 16.0f);
            }
            else
            {
                mLevel->playLocalSound(mX, mY, mZ, SoundEvents::BUBBLE_COLUMN_UPWARDS_INSIDE,
                                       SoundSource::eSoundSource_Players, 0.9f, 1.0f, false, 16.0f);
            }
        }

        mSwimSoundPlayed = true;
    }
    else
    {
        mSwimSoundPlayed = false;
    }

    PIXBeginNamedEvent(0.0f, "aiStep");
    aiStep();
    PIXEndNamedEvent();

    double dx = mX - mXo;
    double dz = mZ - mZo;
    float travelled = (float)(dx * dx + dz * dz);
    float bodyRot = mYBodyRot;
    float headTurn = 0.0f;

    mORun = mRun;

    if (travelled > 0.0025000002f)
    {
        travelled = sqrtf(travelled) * 3.0f;

        float wanted = (float)atan2(dz, dx) * 57.295776f - 90.0f;
        float diff = Mth::abs(Mth::wrapDegrees(mYRot) - wanted);

        if (diff > 95.0f && diff < 265.0f)
        {
            bodyRot = wanted - 180.0f;
        }
        else
        {
            bodyRot = wanted;
        }
    }
    else
    {
        travelled = 0.0f;
    }

    if (mAttackAnim > 0.0f)
    {
        bodyRot = mYRot;
    }

    if (!mOnGround)
    {
        travelled = 0.0f;
    }

    mRun = mRun + (travelled - mRun) * 0.3f;

    headTurn = tickHeadTurn(bodyRot, headTurn);

    while (mYRot - mYRotO < -180.0f)
    {
        mYRotO = mYRotO - 360.0f;
    }

    while (mYRot - mYRotO >= 180.0f)
    {
        mYRotO = mYRotO + 360.0f;
    }

    while (mYBodyRot - mYBodyRotO < -180.0f)
    {
        mYBodyRotO = mYBodyRotO - 360.0f;
    }

    while (mYBodyRot - mYBodyRotO >= 180.0f)
    {
        mYBodyRotO = mYBodyRotO + 360.0f;
    }

    while (mXRot - mXRotO < -180.0f)
    {
        mXRotO = mXRotO - 360.0f;
    }

    while (mXRot - mXRotO >= 180.0f)
    {
        mXRotO = mXRotO + 360.0f;
    }

    while (mYHeadRot - mYHeadRotO < -180.0f)
    {
        mYHeadRotO = mYHeadRotO - 360.0f;
    }

    while (mYHeadRot - mYHeadRotO >= 180.0f)
    {
        mYHeadRotO = mYHeadRotO + 360.0f;
    }

    mAnimStep = mAnimStep + headTurn;

    if (isFallFlying())
    {
        ++mFallFlyTicks;
    }
    else
    {
        mFallFlyTicks = 0;
    }

    PIXEndNamedEvent();
}

float LivingEntity::tickHeadTurn(float yBodyRot, float headTurnDelta)
{
    mYBodyRot = mYBodyRot + Mth::wrapDegrees(yBodyRot - mYBodyRot) * 0.3f;

    float diff = Mth::wrapDegrees(mYRot - mYBodyRot);
    bool turnedAround = !(diff >= -90.0f && diff < 90.0f);

    float clamped = diff;

    if (clamped < -75.0f)
    {
        clamped = -75.0f;
    }

    if (clamped >= 75.0f)
    {
        clamped = 75.0f;
    }

    mYBodyRot = mYRot - clamped;

    if (clamped * clamped > 2500.0f)
    {
        mYBodyRot = mYBodyRot + clamped * 0.2f;
    }

    if (turnedAround)
    {
        headTurnDelta = -headTurnDelta;
    }

    return headTurnDelta;
}

void LivingEntity::updateFallFlying()
{
    bool flying = getSharedFlag(FLAG_FALL_FLYING);

    bool blocked = false;

    if (isInWater() && gApp.GetFirstMiniGameType() != eMiniGameId_Glide)
    {
        blocked = true;
    }

    if (!flying || mOnGround || blocked || isPassenger())
    {
        flying = false;
    }
    else
    {
        not_null_ptr<ItemInstance> chest = getItemSlot(EquipmentSlot::CHEST);
        bool usable = false;

        if (chest->getItem() == Items::elytra && ElytraItem::isFlyEnabled(chest))
        {
            usable = true;
        }

        if (usable)
        {
            flying = true;

            if (!mLevel->mIsClientSide && (mFallFlyTicks + 1) % 20 == 0)
            {
                chest->hurtAndBreak(1, boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
            }
        }
        else
        {
            flying = false;
        }
    }

    if (!mLevel->mIsClientSide)
    {
        if (!flying && gApp.GetFirstMiniGameType() == eMiniGameId_Glide && TakeGlideCollisionDamage())
        {
            float health = getHealth();

            if (hurt(DamageSource::FLY_INTO_WALL, health + 1.0f))
            {
                playSound(getFallDamageSound((int)(health + 1.0f)), 1.0f, 1.0f);
            }
        }

        setSharedFlag(FLAG_FALL_FLYING, flying);
    }
}

void LivingEntity::aiStep()
{
    if (mNoJumpDelay > 0)
    {
        --mNoJumpDelay;
    }

    bool effectiveAi = isEffectiveAi();

    if (mLerpSteps > 0 && !isControlledByLocalInstance())
    {
        double lerpX = mX + (mLerpX - mX) / (double)mLerpSteps;
        double lerpY = mY + (mLerpY - mY) / (double)mLerpSteps;
        double lerpZ = mZ + (mLerpZ - mZ) / (double)mLerpSteps;

        double yRotDelta = Mth::wrapDegrees(mLerpYRot - (double)mYRot);
        double xRotDelta = Mth::wrapDegrees(mLerpXRot - (double)mXRot);

        mYRot = mYRot + (float)(yRotDelta / (double)mLerpSteps);
        mXRot = mXRot + (float)(xRotDelta / (double)mLerpSteps);

        --mLerpSteps;

        setPos(lerpX, lerpY, lerpZ);
        setRot(mYRot, mXRot);

        if (!mNoPhysics)
        {
            AABB* checkBox = mBoundingBox->shrink(0.1, 0.0, 0.1);
            checkBox->mMaxY = checkBox->mMinY + 0.1;

            std::vector<AABB*>* collisions = mLevel->getCollisionAABBs(shared_from_this(), checkBox, false, false,
                                                                       false);

            if (!collisions->empty())
            {
                double minY = mBoundingBox->mMinY;
                double pushY = minY;

                for (std::vector<AABB*>::iterator it = collisions->begin(); it != collisions->end(); ++it)
                {
                    double aabbMaxY = (*it)->mMaxY;

                    if ((aabbMaxY - minY < 0.699999988079071) && (pushY < aabbMaxY))
                    {
                        pushY = aabbMaxY;
                    }
                }

                setPos(lerpX, lerpY + (pushY - minY), lerpZ);
            }
        }
    }
    else if (!effectiveAi)
    {
        mXd = mXd * 0.98;
        mYd = mYd * 0.98;
        mZd = mZd * 0.98;
    }

    if (fabs(mXd) < 5e-05)
    {
        mXd = 0.0;
    }

    if (fabs(mYd) < 5e-05)
    {
        mYd = 0.0;
    }

    if (fabs(mZd) < 5e-05)
    {
        mZd = 0.0;
    }


    if (isImmobile())
    {
        mJumping = false;
        mXxa = 0.0f;
        mZza = 0.0f;
        mYRota = 0.0f;
    }
    else
    {
        MemSect(25);

        if (effectiveAi)
        {
            PIXBeginNamedEvent(0.0f, "serverAiStep");
            serverAiStep();
            PIXEndNamedEvent();
        }

        MemSect(0);
    }


    if (mJumping)
    {
        if (isInWater())
        {
            jumpInWater();
        }
        else if (isInLava())
        {
            jumpInLava();
        }
        else if (mOnGround && mNoJumpDelay == 0)
        {
            jumpFromGround();
            mNoJumpDelay = 10;
        }
    }
    else
    {
        mNoJumpDelay = 0;
    }


    mXxa = mXxa * 0.98f;
    mZza = mZza * 0.98f;
    mYRota = mYRota * 0.9f;

    PIXBeginNamedEvent(0.0f, "travel");
    travel(mXxa, mYya, mZza);

    if (!gApp.IsAppPaused())
    {
        updateFallFlying();
    }

    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Push entities");

    if (!Minecraft::InMiniGame(eMiniGameId_Glide, false))
    {
        pushEntities();
    }

    PIXEndNamedEvent();
}

void LivingEntity::serverAiStep()
{
}

void LivingEntity::pushEntities()
{
    PushableBySelector selector(shared_from_this());
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        shared_from_this(), mBoundingBox->grow(0.20000000298023224, 0.0, 0.20000000298023224), &selector, NULL);

    if (entities == NULL || entities->empty())
    {
        return;
    }

    int maxCramming = mLevel->getGameRules()->getInt(GameRules::RULE_MAX_ENTITY_CRAMMING);

    if (maxCramming > 0 && entities->size() >= (unsigned int)maxCramming && mRandom->nextInt(4) == 0)
    {
        int passengers = 0;

        for (unsigned int i = 0; i < entities->size(); ++i)
        {
            if (!entities->at(i)->isPassenger())
            {
                ++passengers;
            }
        }

        if (passengers > maxCramming - 1)
        {
            hurt(DamageSource::CRAMMING, 6.0f);
        }
    }

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        entity->push(shared_from_this());
    }
}

void LivingEntity::doPush(boost::shared_ptr<Entity> entity)
{
    entity->push(shared_from_this());
}

void LivingEntity::rideTick()
{
    Entity::rideTick();

    mORun = mRun;
    mRun = 0.0f;
    mFallDistance = 0.0f;
}

void LivingEntity::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    mLerpX = x;
    mLerpY = y;
    mLerpZ = z;
    mLerpYRot = (double)yRot;
    mLerpXRot = (double)xRot;
    mLerpSteps = steps;
}

void LivingEntity::setJumping(bool jumping)
{
    mJumping = jumping;
}

void LivingEntity::take(boost::shared_ptr<Entity> entity, int count)
{
    if (!entity->mRemoved && !mLevel->mIsClientSide)
    {
        EntityTracker* tracker = static_cast<ServerLevel*>(mLevel)->getTracker();

        if (eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType())
            || eTYPE_DERIVED_FROM(EntityType::EXPERIENCE_ORB, entity->GetType()))
        {
            tracker->broadcast(entity, boost::shared_ptr<ClientboundTakeItemEntityPacket>(
                                           new ClientboundTakeItemEntityPacket(entity->getId(), getId(), count)));
        }
    }
}

bool LivingEntity::canSee(boost::shared_ptr<Entity> entity)
{
    Vec3* from = Vec3::newTemp(mX, mY + (double)getEyeHeight(), mZ);
    Vec3* to = Vec3::newTemp(entity->mX, entity->mY + (double)entity->getEyeHeight(), entity->mZ);

    HitResult* hit = mLevel->clip(from, to);

    if (hit != NULL)
    {
        delete hit;
        return false;
    }

    return true;
}

Vec3* LivingEntity::getViewVector(float partialTicks)
{
#ifdef MINECRAFT_PC
    double aimEye[3];
    double aimView[3];
    if (PcModViewsAimFor(this, aimEye, aimView))
        return Vec3::newTemp(aimView[0], aimView[1], aimView[2]);
#endif
    if (partialTicks == 1.0f)
    {
        return calculateViewVector(mXRot, mYHeadRot);
    }

    return calculateViewVector(mXRotO + (mXRot - mXRotO) * partialTicks,
                               mYHeadRotO + (mYHeadRot - mYHeadRotO) * partialTicks);
}

float LivingEntity::getAttackAnim(float partialTicks)
{
    float delta = mAttackAnim - mOAttackAnim;

    if (delta < 0.0f)
    {
        delta = delta + 1.0f;
    }

    return mOAttackAnim + delta * partialTicks;
}

AABB* LivingEntity::getSweptVolume()
{
    return AABB::newTemp(mX, mY, mZ, mXOld, mYOld, mZOld);
}

bool LivingEntity::isEffectiveAi()
{
    return !mLevel->mIsClientSide;
}

bool LivingEntity::isPickable()
{
    return !mRemoved;
}

bool LivingEntity::isPushable(boost::shared_ptr<Entity> entity)
{
    return isAlive() && !onLadder();
}

void LivingEntity::markHurt()
{
    mHurtMarked = getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->getValue() <= mRandom->nextDouble();
}

float LivingEntity::getYHeadRot()
{
    return mYHeadRot;
}

void LivingEntity::setYHeadRot(float yHeadRot)
{
    mYHeadRot = yHeadRot;
}

void LivingEntity::setYBodyRot(float yBodyRot)
{
    mYBodyRot = yBodyRot;
}

float LivingEntity::getAbsorptionAmount()
{
    return mAbsorptionAmount;
}

void LivingEntity::setAbsorptionAmount(float absorptionAmount)
{
    if (absorptionAmount < 0.0f)
    {
        absorptionAmount = 0.0f;
    }

    mAbsorptionAmount = absorptionAmount;
}

void LivingEntity::onEnterCombat()
{
}

void LivingEntity::onLeaveCombat()
{
}

void LivingEntity::updateEffectVisibility()
{
    mEffectsDirty = true;
}

bool LivingEntity::isUsingItem()
{
    return (mEntityData->get(DATA_LIVING_ENTITY_FLAGS) & 1) != 0;
}

InteractionHand::EInteractionHand LivingEntity::getUsedItemHand()
{
    return (mEntityData->get(DATA_LIVING_ENTITY_FLAGS) & 2) != 0 ? InteractionHand::eInteractionHand_OffHand
                                                                 : InteractionHand::eInteractionHand_MainHand;
}

int LivingEntity::getUseItemRemainingTicks()
{
    return mUseItemRemaining;
}

void LivingEntity::updatingUsingItem()
{
    if (!isUsingItem())
    {
        return;
    }

    if (getItemInHand(getUsedItemHand()) == mUseItem)
    {
        int remaining = getUseItemRemainingTicks();

        if (remaining < 26 && (remaining & 3) == 0)
        {
            spawnItemUseParticles(mUseItem, 5);
        }

        --mUseItemRemaining;

        if (mUseItemRemaining == 0 && !mLevel->mIsClientSide)
        {
            completeUsingItem();
        }
    }
    else
    {
        stopUsingItem();
    }
}

void LivingEntity::startUsingItem(InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = getItemInHand(hand);

    if (itemInstance->isEmpty() || isUsingItem())
    {
        return;
    }

    mUseItem = itemInstance;
    mUseItemRemaining = itemInstance->getUseDuration();

    if (!mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_LIVING_ENTITY_FLAGS,
                         (unsigned char)(hand == InteractionHand::eInteractionHand_OffHand ? 3 : 1), true);
    }
}

void LivingEntity::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Entity::onSyncedDataUpdated(accessor);

    if (DATA_LIVING_ENTITY_FLAGS->getId() == accessor->getId() && mLevel->mIsClientSide)
    {
        if (isUsingItem() && mUseItem->isEmpty())
        {
            mUseItem = getItemInHand(getUsedItemHand());

            if (!mUseItem->isEmpty())
            {
                mUseItemRemaining = mUseItem->getUseDuration();
            }
        }
        else if (!isUsingItem() && !mUseItem->isEmpty())
        {
            mUseItem = ItemInstance::EMPTY;
            mUseItemRemaining = 0;
        }
    }
}

void LivingEntity::spawnItemUseParticles(not_null_ptr<ItemInstance> itemInstance, int count)
{
    if (itemInstance->isEmpty() || !isUsingItem())
    {
        return;
    }

    if (itemInstance->getUseAnimation() == UseAnim_drink)
    {
        playSound(SoundEvents::GENERIC_DRINK, 0.5f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f);
    }

    if (itemInstance->getUseAnimation() != UseAnim_eat)
    {
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        Vec3* velocity = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0.0);
        velocity = velocity->xRot(-(mXRot * Mth::DEG_TO_RAD));
        velocity = velocity->yRot(-(mYRot * Mth::DEG_TO_RAD));

        Vec3* offset = Vec3::newTemp((mRandom->nextFloat() - 0.5) * 0.3, -mRandom->nextFloat() * 0.6 - 0.3, 0.6);
        offset = offset->xRot(-(mXRot * Mth::DEG_TO_RAD));
        offset = offset->yRot(-(mYRot * Mth::DEG_TO_RAD));
        offset = offset->add(mX, mY + (double)getEyeHeight(), mZ);

        if (itemInstance->isStackedByData())
        {
            int particleData[2] = {Item::getId(itemInstance->getItem()), itemInstance->getAuxValue()};
            mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z, velocity->x, velocity->y + 0.05,
                                velocity->z, arrayWithLength<int>(particleData, 2));
        }
        else
        {
            int particleData[1] = {Item::getId(itemInstance->getItem())};
            mLevel->addParticle(PARTICLE_ICON_CRACK, offset->x, offset->y, offset->z, velocity->x, velocity->y + 0.05,
                                velocity->z, arrayWithLength<int>(particleData, 1));
        }
    }

    playSound(SoundEvents::GENERIC_EAT, 0.5f + 0.5f * (float)mRandom->nextInt(2),
              (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f + 1.0f);
}

void LivingEntity::completeUsingItem()
{
    if (mUseItem->isEmpty() || !isUsingItem())
    {
        return;
    }

    spawnItemUseParticles(mUseItem, 16);

    setItemInHand(getUsedItemHand(),
                  mUseItem->finishUsingItem(mLevel, boost::dynamic_pointer_cast<Player>(shared_from_this())));
    stopUsingItem();
}

int LivingEntity::getTicksUsingItem()
{
    if (!isUsingItem() || mUseItem->isEmpty())
    {
        return 0;
    }

    return mUseItem->getUseDuration() - getUseItemRemainingTicks();
}

void LivingEntity::releaseUsingItem()
{
    releaseUsingItem(getUseItemRemainingTicks());
}

void LivingEntity::releaseUsingItem(int useDuration)
{
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();

    if (!mUseItem->isEmpty() && (gameMode == NULL || gameMode->IsValidStateForItemUse()))
    {
        mUseItem->releaseUsing(mLevel, boost::static_pointer_cast<LivingEntity>(shared_from_this()), useDuration);
    }

    stopUsingItem();
}

void LivingEntity::stopUsingItem()
{
    if (!mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_LIVING_ENTITY_FLAGS, (unsigned char)0, true);
    }

    mUseItem = ItemInstance::EMPTY;
    mUseItemRemaining = 0;
}

bool LivingEntity::isBlocking()
{
    if (!isUsingItem() || mUseItem->isEmpty())
    {
        return false;
    }

    return mUseItem->getItem()->getUseAnimation(not_null_ptr<const ItemInstance>(mUseItem)) == UseAnim_block;
}

bool LivingEntity::isFallFlying()
{
    return getSharedFlag(FLAG_FALL_FLYING);
}

int LivingEntity::getFallFlyingTicks()
{
    return mFallFlyTicks;
}

bool LivingEntity::getElytraProximityWarning()
{
    return mElytraProximityWarning;
}

double LivingEntity::getElytraCollisionDistance()
{
    return mElytraCollisionDistance;
}

not_null_ptr<ItemInstance> LivingEntity::getUseItem()
{
    return mUseItem;
}

bool LivingEntity::randomTeleport(double x, double y, double z)
{
    double oldX = mX;
    double oldY = mY;
    double oldZ = mZ;

    mX = x;
    mY = y;
    mZ = z;

    BlockPos pos(this);
    Level* level = mLevel;
    Random* random = getRandom();

    bool teleported = false;

    if (level->hasChunkAt(pos) && pos.y > 0)
    {
        bool grounded = true;

        BlockPos below = pos.below();

        while (!level->getBlockState(mTickCache, below)->getMaterial()->blocksMotion())
        {
            mY = mY - 1.0;
            pos = below;

            if (pos.y < 1)
            {
                grounded = false;
                break;
            }

            below = pos.below();
        }

        if (grounded)
        {
            Entity::setPos(mX, mY, mZ);
            AABB* box = mBoundingBox;
            if (level->getCollisionAABBs(shared_from_this(), box, false, false, false)->empty()
                && !level->containsAnyLiquid(box))
            {
                teleported = true;
            }
        }
    }

    if (teleported)
    {
        teleportTo(mX, mY, mZ);

        for (int i = 0; i < 128; ++i)
        {
            double progress = (double)i / 127.0;

            float xd = (random->nextFloat() - 0.5f) * 0.2f;
            float yd = (random->nextFloat() - 0.5f) * 0.2f;
            float zd = (random->nextFloat() - 0.5f) * 0.2f;

            double px = oldX + (mX - oldX) * progress + (random->nextDouble() - 0.5) * (double)mBbWidth * 2.0;
            double py = oldY + (mY - oldY) * progress + random->nextDouble() * (double)mBbHeight;
            double pz = oldZ + (mZ - oldZ) * progress + (random->nextDouble() - 0.5) * (double)mBbWidth * 2.0;

            level->addParticle(PARTICLE_PORTAL, px, py, pz, (double)xd, (double)yd, (double)zd, arrayWithLength<int>());
        }

        if (eTYPE_DERIVED_FROM(EntityType::PATHFINDER_MOB, GetType()))
        {
            static_cast<PathfinderMob*>(this)->getNavigation()->stop();
        }

        return true;
    }

    Entity::setPos(oldX, oldY, oldZ);
    return false;
}

bool LivingEntity::isAffectedByPotions()
{
    return true;
}

bool LivingEntity::attackable()
{
    return true;
}

void LivingEntity::setRecordPlayingNearby(const BlockPos& pos, bool playing)
{
}

Team* LivingEntity::getTeam()
{
    if (GetType() == EntityType::PLAYER)
    {
        return mLevel->GetScoreboard()->getPlayersTeam(static_cast<Player*>(this)->mXuid);
    }

    return NULL;
}

void LivingEntity::disableFallFlyingState(bool disabled)
{
    mFallFlyingStateDisabled = disabled;
}

bool LivingEntity::isFallFlyingStateDisabled()
{
    return mFallFlyingStateDisabled;
}

void LivingEntity::stopCurrentLerp()
{
    Entity::stopCurrentLerp();

    mYHeadRotO = mYHeadRot;
    mYBodyRotO = mYBodyRot;
    mAnimationSpeed = 0.0f;
    mAnimationSpeedOld = 0.0f;
}

bool LivingEntity::IsCreativeFlying()
{
    return false;
}

unsigned long long LivingEntity::GetType()
{
    return EntityType::LIVING_ENTITY;
}
