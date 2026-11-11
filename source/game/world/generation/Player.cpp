#include "game/world/generation/Player.hpp"
#include "game/core/Minecart.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/Pig.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/core/Difficulty.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/network/ClientboundSetEntityMotionPacket.hpp"
#include "game/entity/mob/MultiEntityMobPart.hpp"
#include "game/entity/mob/MultiEntityMob.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/DLCSkinFile.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/GameType.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/CombatTracker.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/client/audio/SoundEngine.hpp"

#include "game/world/Level.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/GenericStats.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/BedBlock.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/Math.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/PlayerEnderChestContainer.hpp"
#include "game/client/render/PlayerModelPart.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemId.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/InsomniaComponent.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/MemSect.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/core/MOJANG_DATA.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/LockCode.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/Team.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/world/generation/PlayerDataWalker.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/client/audio/ElytraOnPlayerSoundInstance.hpp"
#include "game/core/Parrot.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/FishingRodItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/AxeItem.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include <boost/functional/hash.hpp>
#include <climits>
#include <cmath>
#include <iomanip>
#include <sstream>

const EntityDataAccessor<float>* Player::DATA_PLAYER_ABSORPTION_ID = NULL;
const EntityDataAccessor<int>* Player::DATA_SCORE_ID = NULL;
const EntityDataAccessor<unsigned char>* Player::DATA_PLAYER_MODE_CUSTOMISATION = NULL;
const EntityDataAccessor<unsigned char>* Player::DATA_MAIN_HAND = NULL;
const EntityDataAccessor<CompoundTag*>* Player::DATA_SHOULDER_LEFT = NULL;
const EntityDataAccessor<CompoundTag*>* Player::DATA_SHOULDER_RIGHT = NULL;
std::vector<unsigned int> Player::sDlcSkinIds;

Player::ChatVisiblity::ChatVisiblity(int id) : mId(id)
{
}

Player::PreventsMeFromSleeping::PreventsMeFromSleeping(boost::shared_ptr<Player> player)
{
    mPlayer = player;
}

bool Player::PreventsMeFromSleeping::apply(boost::shared_ptr<Entity> entity) const
{
    boost::shared_ptr<Monster> monster = boost::dynamic_pointer_cast<Monster>(entity);
    return monster != NULL && monster->isPreventingPlayerRest(mPlayer);
}

Player::Player(Level* level, const std::wstring& name) : LivingEntity(level), mAbilities()
{
    _init();
    mName = name;
    MemSect(11);
    mInventoryMenu = new InventoryMenu(mInventory, !level->mIsClientSide, this);
    MemSect(0);
    mContainerMenu = mInventoryMenu;
    BlockPos spawn = level->getSharedSpawnPos();
    moveTo(spawn.x + 0.5, spawn.y + 1.0, spawn.z + 0.5, 0.0f, 0.0f);
    mField0x7d8 = 1;
    mCustomSkin = 0;
    mCustomCape = 0;
    mPlayerIndex = 0;
    mPlayedFallingSound = false;
    mLastPoints = 0;
    mLastCheckpointId = 0;
    mEndZoneEffectShown = 0;
    mRotOffs = 180.0f;
    SetPowerupTicks(PowerupItems::eGlide_Timed_Powerup_ID_SlowFall, 0);
    SetPowerupTicks(PowerupItems::eGlide_Timed_Powerup_ID_InvertControls, 0);
    SetPowerupTicks(PowerupItems::eGlide_Timed_Powerup_ID_Invulnerable, 0);
    mXuid = PlayerUID(0);
    mOnlineXuid = PlayerUID(0);
    mSuid = StatsUID::EMPTY_SUID();
    mShowOnMaps = gApp.GetGameHostOption(eGameHostOption_Gamertags) != 0;
    mIsGuest = false;
    setUUID(name);
}

StatsUID Player::getSUID()
{
    if (mSuid == StatsUID::EMPTY_SUID())
        mSuid = StatsUID(mXuid);
    return mSuid;
}

Player::~Player()
{
    delete mInventoryMenu;
    mInventoryMenu = NULL;
    if (mBedPosition == mRespawnPosition)
        mRespawnPosition = NULL;
    delete mBedPosition;
    mBedPosition = NULL;
    delete mRespawnPosition;
    mRespawnPosition = NULL;
    if (mOwnsCameraController)
        delete mCameraController;
    if (mField0x7c8.size() != 0)
        delete[] &mField0x7c8[0];
    if (mField0x76c.size() != 0)
        delete[] &mField0x76c[0];
    delete mField0x738;
}

bool Player::shouldTick()
{
    return true;
}

void Player::registerAttributes()
{
    LivingEntity::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(1.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.10000000149011612);
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_SPEED);
    getAttributes()->registerAttribute(SharedMonsterAttributes::LUCK);
}

void Player::staticCtor()
{
    DATA_PLAYER_ABSORPTION_ID = SynchedEntityData::defineId(EntityType::PLAYER, EntityType::LIVING_ENTITY,
                                                            EntityDataSerializers::FLOAT);
    DATA_SCORE_ID = SynchedEntityData::defineId(EntityType::PLAYER, EntityType::LIVING_ENTITY,
                                                EntityDataSerializers::INT);
    DATA_PLAYER_MODE_CUSTOMISATION = SynchedEntityData::defineId(EntityType::PLAYER, EntityType::LIVING_ENTITY,
                                                                 EntityDataSerializers::BYTE);
    DATA_MAIN_HAND = SynchedEntityData::defineId(EntityType::PLAYER, EntityType::LIVING_ENTITY,
                                                 EntityDataSerializers::BYTE);
    DATA_SHOULDER_LEFT = SynchedEntityData::defineId(EntityType::PLAYER, EntityType::LIVING_ENTITY,
                                                     EntityDataSerializers::COMPOUND_TAG);
    DATA_SHOULDER_RIGHT = SynchedEntityData::defineId(EntityType::PLAYER, EntityType::LIVING_ENTITY,
                                                      EntityDataSerializers::COMPOUND_TAG);

    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00000107"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin20000107"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00000111"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin20000111"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00000222"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin20000222"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00000223"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin20000223"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin20000231"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00000320"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002056"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002057"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002518"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002715"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002716"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00001626"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00001244"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00001246"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00001248"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00001254"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002405"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002417"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002421"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002411"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002424"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00003225"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002730"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002715"));
    sDlcSkinIds.push_back(CMinecraftApp::getSkinIdFromPath(L"dlcskin00002104"));
}

void Player::defineSynchedData()
{
    LivingEntity::defineSynchedData();
    mEntityData->createDateItem(DATA_PLAYER_ABSORPTION_ID, 0.0f);
    mEntityData->define(DATA_SCORE_ID, 0);
    mEntityData->define(DATA_PLAYER_MODE_CUSTOMISATION, static_cast<unsigned char>(0));
    mEntityData->define(DATA_MAIN_HAND, static_cast<unsigned char>(1));
    mEntityData->define(DATA_SHOULDER_LEFT, new CompoundTag());
    mEntityData->define(DATA_SHOULDER_RIGHT, new CompoundTag());
}

bool Player::shouldRenderAtSqrDistance(double distance)
{
    double range = mBoundingBox->getSize() * 10.0;
    if (range != range)
        range = 1.0;
    range *= 64.0 * getViewScale();
    return distance < range * range;
}

bool Player::checkBedExists()
{
    return mLevel->getBlockState(*mBedPosition)->getBlock() == Blocks::bed;
}

void Player::moveCloak()
{
    double cloakX = mX - mChasingPosX;
    double cloakY = mY - mChasingPosY;
    double cloakZ = mZ - mChasingPosZ;

    if (cloakX > 10.0 || cloakX < -10.0)
        mChasingPosX = mX;
    if (cloakY > 10.0 || cloakY < -10.0)
        mChasingPosY = mY;
    if (cloakZ > 10.0 || cloakZ < -10.0)
        mChasingPosZ = mZ;

    mPrevChasingPosX = mChasingPosX;
    mPrevChasingPosY = mChasingPosY;
    mPrevChasingPosZ = mChasingPosZ;
    mChasingPosX += cloakX * 0.25;
    mChasingPosY += cloakY * 0.25;
    mChasingPosZ += cloakZ * 0.25;
}

void Player::SetNormalSize()
{
    mField0x828 = isSpectator();
}

void Player::resetPos()
{
    SetNormalSize();
    Entity::resetPos();
    setHealth(getMaxHealth());
    mDeathTime = 0;
}

void Player::handleEntityEvent(uint8_t event, int32_t data)
{
    if (event == 9)
    {
        completeUsingItem();
    }
    else if (event == 22)
    {
        mReducedDebugInfo = 1;
    }
    else if (event == 23)
    {
        mReducedDebugInfo = 0;
    }
    else
    {
        LivingEntity::handleEntityEvent(event, data);
    }
}

uint32_t Player::canHarmPlayer(boost::shared_ptr<Player> player)
{
    Team* team = getTeam();
    Team* otherTeam = player->getTeam();
    if (team != NULL && team->isAlliedTo(otherTeam))
        return team->isAllowFriendlyFire();
    return 1;
}

uint32_t Player::getScore()
{
    return mEntityData->get(DATA_SCORE_ID);
}

void Player::setScore(int32_t score)
{
    mEntityData->set(DATA_SCORE_ID, score, true);
}

void Player::increaseScore(int32_t amount)
{
    int32_t score = static_cast<int32_t>(getScore());
    mEntityData->set(DATA_SCORE_ID, score + amount, true);
}

CompoundTag* Player::getShoulderEntityLeft()
{
    return mEntityData->get(DATA_SHOULDER_LEFT);
}

CompoundTag* Player::getShoulderEntityRight()
{
    return mEntityData->get(DATA_SHOULDER_RIGHT);
}

void Player::setShoulderEntityLeft(CompoundTag* tag)
{
    mEntityData->set(DATA_SHOULDER_LEFT, tag, true);
}

void Player::setShoulderEntityRight(CompoundTag* tag)
{
    mEntityData->set(DATA_SHOULDER_RIGHT, tag, true);
}

uint32_t Player::setEntityOnShoulder(CompoundTag* tag)
{
    if (isPassenger() || !mOnGround || isInWater())
        return 0;
    if (getShoulderEntityLeft()->isEmpty())
    {
        setShoulderEntityLeft(tag);
        return 1;
    }
    if (getShoulderEntityRight()->isEmpty())
    {
        setShoulderEntityRight(tag);
        return 1;
    }
    delete tag;
    return 0;
}

bool Player::canHarmPlayer(std::wstring)
{
    return isAllowedToAttackPlayers();
}

unsigned int Player::getPlayerGamePrivilege(unsigned int privileges, EPlayerGamePrivileges privilege)
{
    if (privilege == ePlayerGamePrivilege_All)
        return privileges;
    if (privilege >= 32)
        return 0;
    return privileges & (1u << privilege);
}

unsigned int Player::getPlayerGamePrivilege(EPlayerGamePrivileges privilege)
{
    return getPlayerGamePrivilege(mGamePrivileges, privilege);
}

uint32_t Player::hasInvulnerablePrivilege()
{
    return gApp.GetGameHostOption(eGameHostOption_HostCanBeInvisible) != 0
           && getPlayerGamePrivilege(ePlayerGamePrivilege_Invulnerable) != 0;
}

uint32_t Player::isAllowedToFly()
{
    return gApp.GetGameHostOption(eGameHostOption_HostCanFly) != 0
           && getPlayerGamePrivilege(ePlayerGamePrivilege_CanFly) != 0;
}

uint32_t Player::hasInvisiblePrivilege()
{
    return gApp.GetGameHostOption(eGameHostOption_HostCanBeInvisible) != 0
           && getPlayerGamePrivilege(ePlayerGamePrivilege_Invisible) != 0;
}

uint32_t Player::isAllowedToAttackPlayers()
{
    return !hasInvisiblePrivilege() && gApp.GetGameHostOption(eGameHostOption_PvP) != 0
           && (gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0
               || getPlayerGamePrivilege(ePlayerGamePrivilege_CannotAttackPlayers) == 0);
}

uint32_t Player::isAllowedToIgnoreExhaustion()
{
    return gApp.GetGameHostOption(eGameHostOption_HostCanChangeHunger) != 0
           && getPlayerGamePrivilege(ePlayerGamePrivilege_ExhaustionDisabled) != 0;
}

void Player::causeFoodExhaustion(float exhaustion)
{
    if (isAllowedToIgnoreExhaustion())
        return;
    if (isAllowedToFly() && mAbilities.mFlying)
        return;
    if (mAbilities.mInvulnerable || hasInvulnerablePrivilege())
        return;
    if (gApp.GetGameHostOption(eGameHostOption_TrustPlayers) == 0
        && getPlayerGamePrivilege(ePlayerGamePrivilege_CannotBuild) != 0)
        return;
    if (!mLevel->mIsClientSide)
        mFoodData.addExhaustion(exhaustion);
}

uint32_t Player::isAllowedToMine()
{
    return gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0
           || getPlayerGamePrivilege(ePlayerGamePrivilege_CannotMine) == 0;
}

bool Player::isAllowedToAttackAnimals()
{
    bool allowed = true;
    if (gApp.GetGameHostOption(eGameHostOption_TrustPlayers) == 0
        && getPlayerGamePrivilege(ePlayerGamePrivilege_CannotAttackAnimals) != 0)
    {
        allowed = false;
    }
    return allowed;
}

bool Player::canCreateParticles()
{
    return !hasInvisiblePrivilege();
}

int Player::GetSuffocateTime()
{
    return -20;
}

float Player::GetInhaleTime()
{
    return 3.75f;
}

bool Player::generatesBubbles()
{
    return false;
}

int Player::getDimensionChangingDelay()
{
    return 10;
}

SoundSource::ESoundSource Player::getSoundSource()
{
    return SoundSource::eSoundSource_Players;
}

double Player::getRidingHeight()
{
    return -0.3499999940395355;
}

void Player::stopRiding()
{
    LivingEntity::stopRiding();
    mBoardingCooldown = 0;
}

void Player::SetSmallSize()
{
    mField0x828 = 1;
}

void Player::SetNoCollisionSize()
{
    mField0x828 = 2;
}

void Player::updatePlayerSize()
{
    float width;
    float height;
    if (mField0x828 == 1)
    {
        width = 0.3f;
        height = 0.3f;
    }
    else if (mField0x828 == 2)
    {
        width = 0.0f;
        height = 0.0f;
    }
    else if (isFallFlying())
    {
        width = 0.6f;
        height = 0.6f;
    }
    else if (isSleeping())
    {
        width = 0.2f;
        height = 0.2f;
    }
    else if (isSwimming() || getDamageNearbyMobs())
    {
        width = 0.6f;
        height = 0.6f;
    }
    else
    {
        width = 0.6f;
        height = isSneaking() ? 1.65f : 1.8f;
    }

    if (width == mBbWidth && height == mBbHeight)
    {
        return;
    }

    AABB* resized = AABB::newTemp(mBoundingBox->mMinX, mBoundingBox->mMinY, mBoundingBox->mMinZ,
                                  mBoundingBox->mMinX + width, mBoundingBox->mMinY + height,
                                  mBoundingBox->mMinZ + width);
    if (!mLevel->hasBlockCubes(resized) || isPassenger())
    {
        setSize(width, height);
    }
}

void Player::playSound(const SoundEvent* sound, float volume, float pitch)
{
    mLevel->playSound(boost::static_pointer_cast<Player>(shared_from_this()), mX, mY, mZ, sound, getSoundSource(),
                      volume, pitch, 16.0f);
}

uint64_t Player::GetType()
{
    return EntityType::PLAYER;
}

int Player::GetThirdPersonView(bool)
{
    return 0;
}

void Player::SetThirdPersonView(int)
{
}

uint32_t Player::CanChangeThirdPersonView()
{
    return 0;
}

bool Player::PositionLocked()
{
    return mPositionLocked;
}

void Player::SetPositionLocked(bool locked)
{
    mPositionLocked = locked;
}

bool Player::TakeGlideCollisionDamage()
{
    return mField0x815;
}

void Player::SetGlideCollisionDamage(bool enabled)
{
    mField0x815 = enabled;
}

double Player::GetLiftForceModifier()
{
    return mField0x818;
}

void Player::SetLiftForceModifier(double modifier)
{
    mField0x818 = modifier;
}

GameMode* Player::GetGameMode()
{
    return 0;
}

void Player::setPlayerDefaultSkin(EDefaultSkins skin)
{
    mField0x7d8 = skin;
}

void Player::SetAdditionalModelParts(std::vector<ModelPart*>* parts)
{
    mAdditionalModelParts = parts;
}

void Player::Follow(bool follow)
{
    if (!follow)
        mPendingCameraId = -1;
}

double Player::getUnderwaterLightLevel() const
{
    return static_cast<double>(mField0x854);
}

void Player::setUnderwaterLightLevel(float level)
{
    mField0x854 = level;
}

double Player::getUnderwaterVisionScale() const
{
    return static_cast<double>(mField0x858);
}

void Player::setUnderwaterVisionScale(float scale)
{
    mField0x858 = scale;
}

void Player::onCrafted(not_null_ptr<ItemInstance>)
{
}

void Player::setPlayerGamePrivilege(uint32_t& privileges, EPlayerGamePrivileges privilege, uint32_t enabled)
{
    if (privilege == ePlayerGamePrivilege_All)
    {
        privileges = enabled;
        return;
    }
    if (privilege == ePlayerGamePrivilege_Moderator)
    {
        if (enabled)
        {
            setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_Op, 1);
            setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleInvisible, 1);
            setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleFly, 1);
            setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleExhaustion, 1);
            privileges |= 1u << ePlayerGamePrivilege_CanTeleport;
            return;
        }
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleInvisible, 0);
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleFly, 0);
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleExhaustion, 0);
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanTeleport, 0);
        privileges &= ~(1u << ePlayerGamePrivilege_Op);
        return;
    }
    if ((int)privilege > 0x1f)
        return;
    if (enabled)
    {
        privileges |= 1u << privilege;
        return;
    }
    switch (privilege)
    {
    case ePlayerGamePrivilege_Op:
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleInvisible, 0);
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleFly, 0);
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanToggleExhaustion, 0);
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanTeleport, 0);
        break;
    case ePlayerGamePrivilege_CanToggleInvisible:
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_Invisible, 0);
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_Invulnerable, 0);
        break;
    case ePlayerGamePrivilege_CanToggleFly:
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanFly, 0);
        break;
    case ePlayerGamePrivilege_CanToggleExhaustion:
        setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_ExhaustionDisabled, 0);
        break;
    default:
        break;
    }
    privileges &= ~(1u << privilege);
}


void Player::setPlayerGamePrivilege(EPlayerGamePrivileges privilege, uint32_t enabled)
{
    setPlayerGamePrivilege(mGamePrivileges, privilege, enabled);
}

bool Player::isModerator()
{
    return getPlayerGamePrivilege(ePlayerGamePrivilege_Op) != 0;
}

void Player::enableAllPlayerPrivileges(uint32_t& privileges, bool enabled)
{
    const uint32_t inverse = enabled ? 0u : 1u;
    setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CannotMine, inverse);
    setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CannotBuild, inverse);
    setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CannotAttackPlayers, inverse);
    setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CannotAttackAnimals, inverse);
    setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanUseDoorsAndSwitches, enabled);
    setPlayerGamePrivilege(privileges, ePlayerGamePrivilege_CanUseContainers, enabled);
}

void Player::enableAllPlayerPrivileges(bool enabled)
{
    enableAllPlayerPrivileges(mGamePrivileges, enabled);
}

void Player::SetLastAteKelp(bool ate)
{
    mLastAteKelp = ate;
    if (!ate)
        ResetTimerAchievement(eTimerAchievement_Castaway);
}

void Player::startSpinAttack()
{
    setDamageNearbyMobs(true);
    updatePlayerSize();
}

void Player::setInvisible(bool invisible)
{
    Entity::setInvisible(invisible);
}

void Player::SpectatePlayerNext()
{
    Follow(true);
}

void Player::SpectatePlayerPrev()
{
    Follow(true);
}

void Player::handleCollectItem(not_null_ptr<ItemInstance>)
{
}

bool Player::ShouldRenderShadow()
{
    return !(isSpectator() && IsSpectatingOtherPlayer());
}

std::wstring Player::getDisplayName()
{
    return mDisplayName.empty() ? mName : mDisplayName;
}

void Player::stopSpinAttack()
{
    setDamageNearbyMobs(false);
    updatePlayerSize();
}

void Player::makeStuckInWeb()
{
    if (!mAbilities.mFlying)
        Entity::makeStuckInWeb();
}

void Player::respawn()
{
    mDeathFadeTimer = 0;
    if (mInsomniaComponent != NULL)
        mInsomniaComponent->restartTimer();
}

int Player::getPortalWaitTime()
{
    return mAbilities.mInvulnerable ? 0 : 80;
}

int Player::getXpNeededForNextLevel()
{
    if (mExperienceLevel >= 30)
        return mExperienceLevel * 9 - 158;
    if (mExperienceLevel >= 15)
        return mExperienceLevel * 5 - 38;
    return mExperienceLevel * 2 + 7;
}

uint32_t Player::getEnchantmentSeed()
{
    return static_cast<uint32_t>(mEnchantmentSeed);
}

bool Player::isLocalPlayer()
{
    return 0;
}

bool Player::isInWall()
{
    return !mSleeping && Entity::isInWall();
}

std::wstring Player::getGameProfile()
{
    return mName;
}

bool Player::bedInRange(const BlockPos& position, const Direction* direction)
{
    if (std::fabs(mX - position.x) <= 3.0 && std::fabs(mY - position.y) <= 2.0 && std::fabs(mZ - position.z) <= 3.0)
        return true;
    BlockPos opposite = position.relative(direction->getOpposite());
    return std::fabs(mX - opposite.x) <= 3.0 && std::fabs(mY - opposite.y) <= 2.0 && std::fabs(mZ - opposite.z) <= 3.0;
}

bool Player::isSleepingLongEnough()
{
    return mSleeping != 0 && mSleepCounter >= 100;
}

int Player::getSleepTimer()
{
    return mSleepCounter;
}

int Player::getDeathFadeTimer()
{
    return mDeathFadeTimer;
}

void Player::displayClientMessage(int, bool)
{
}

BlockPos* Player::getRespawnPosition()
{
    return mRespawnPosition;
}

uint8_t Player::isRespawnForced()
{
    return mRespawnForced;
}

bool Player::isModelPartShown(const PlayerModelPart*)
{
    return true;
}

uint8_t Player::isReducedDebugInfo()
{
    return mReducedDebugInfo;
}

bool Player::isAlwaysExperienceDropper()
{
    return true;
}

bool Player::shouldShowName()
{
    return true;
}

void Player::doWaterSplashEffect()
{
    if (!isSpectator())
        Entity::doWaterSplashEffect();
}

bool Player::isPushedByWater()
{
    return !mAbilities.mFlying;
}

float Player::getAttackStrengthScale(float)
{
    return 1.0f;
}

bool Player::mayBuild()
{
    return mAbilities.mMayBuild;
}

uint8_t Player::getTexture()
{
    static const uint8_t textures[20] = {0,    0,    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x70,
                                         0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0xaa, 0xa9, 0};
    if (mField0x7d8 < 2 || mField0x7d8 > 0x12)
    {
        return 0x25;
    }
    return textures[mField0x7d8];
}

void Player::UpdateSize()
{
    SetNormalSize();
}

void Player::hurtCurrentlyUsedShield(float)
{
}

void Player::disableShield(bool)
{
}

void Player::crit(boost::shared_ptr<Entity>)
{
}

void Player::magicCrit(boost::shared_ptr<Entity>)
{
}

void Player::openTextEdit(boost::shared_ptr<SignBlockEntity>)
{
}

void Player::openMinecartCommandBlock(boost::shared_ptr<BaseCommandBlock>)
{
}

void Player::openCommandBlock(boost::shared_ptr<CommandBlockEntity>)
{
}

int Player::openTrading(boost::shared_ptr<Merchant>, const std::wstring&, int)
{
    return 0;
}

int Player::openContainer(boost::shared_ptr<Container>)
{
    return 0;
}

int Player::openHorseInventory(boost::shared_ptr<AbstractHorse>, boost::shared_ptr<Container>)
{
    return 0;
}

int Player::startBlockInteraction(InteractionObject&)
{
    return 0;
}

void Player::openItemInstanceGui(not_null_ptr<ItemInstance>, InteractionHand::EInteractionHand)
{
}

bool Player::isSleeping()
{
    return mSleeping;
}

uint32_t Player::hasAchievement(Achievement*)
{
    return 0;
}

void Player::resetStat(Stat*)
{
}

void Player::onUpdateAbilities()
{
}

void Player::setGameMode(const GameType*)
{
}

void Player::OnEquipArmor(not_null_ptr<ItemInstance>)
{
}

void Player::OnTakeFromAnvil(not_null_ptr<ItemInstance>)
{
}

uint32_t Player::GetModelTypeFromAnimBitmask(uint32_t value)
{
    if ((value & 0x40000u) != 0)
        return 1;
    if ((value & 0x80000u) == 0)
        return 0;
    return 2;
}

uint32_t Player::GetModelTypeFromTextureId(int32_t value)
{
    switch (value)
    {
    case 0x25:
    case 0xa9:
        return 1;
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0xaa:
        return 2;
    default:
        return 0;
    }
}

void Player::AdvanceTimerAchievement(TimerAchievements achievement, int32_t ticks)
{
    if (ticks < 1)
        return;
    mTimerAchievementTicks[achievement] += ticks;
}

void Player::SetPowerupTicks(PowerupItems::eGlide_Timed_Powerup_ID powerup, int32_t ticks)
{
    mPowerupTicks[powerup] = ticks;
}

bool Player::isStarving()
{
    return mFoodData.isStarving();
}

void Player::resetAttackStrengthTicker()
{
    mAttackStrengthTicker = 0;
}

bool Player::isSendMessageEnabled()
{
    return getServer()->mLevels.data[0]->getGameRules()->getBoolean(GameRules::RULE_SENDCOMMANDFEEDBACK);
}

bool Player::makeStepSound()
{
    return !mAbilities.mFlying;
}

const SoundEvent* Player::getSwimSound()
{
    return SoundEvents::PLAYER_SWIM;
}

const SoundEvent* Player::getSwimSplashSound()
{
    return SoundEvents::PLAYER_SPLASH;
}

const SoundEvent* Player::getHeavySplashSound()
{
    return SoundEvents::PLAYER_SPLASH_HIGH_SPEED;
}

const SoundEvent* Player::getDeathSound()
{
    return SoundEvents::PLAYER_DEATH;
}

FoodData* Player::getFoodData()
{
    return &mFoodData;
}

boost::shared_ptr<PlayerEnderChestContainer> Player::getEnderChestInventory()
{
    return mEnderChestInventory;
}

ItemCooldowns* Player::getCooldowns()
{
    return mField0x738;
}

void Player::hurtArmor(float damage)
{
    mInventory->hurtArmor(damage);
}

float Player::getArmorCoverPercentage()
{
    int worn = 0;
    for (unsigned int slot = 0; slot < mInventory->m_armor.size(); ++slot)
    {
        if (!mInventory->m_armor[slot]->isEmpty())
            ++worn;
    }
    return static_cast<float>(worn) / static_cast<float>(mInventory->m_armor.size());
}

uint8_t Player::canUseCommandBlocks()
{
    return mAbilities.mInstabuild;
}

not_null_ptr<ItemInstance> Player::getItemSlot(const EquipmentSlot* slot)
{
    if (slot == EquipmentSlot::MAINHAND)
        return mInventory->getSelected();
    if (slot == EquipmentSlot::OFFHAND)
        return mInventory->m_offhand[0];
    if (slot->getType() == EquipmentSlot::ARMOR)
        return mInventory->m_armor[slot->getIndex()];
    return ItemInstance::EMPTY;
}

void Player::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item)
{
    if (slot == EquipmentSlot::MAINHAND)
    {
        playEquipSound(item);
        mInventory->m_items[mInventory->m_selected] = item;
    }
    else if (slot == EquipmentSlot::OFFHAND)
    {
        playEquipSound(item);
        mInventory->m_offhand[0] = item;
    }
    else if (slot->getType() == EquipmentSlot::ARMOR)
    {
        playEquipSound(item);
        mInventory->m_armor[slot->getIndex()] = item;
    }
}

uint32_t Player::addItem(const not_null_ptr<ItemInstance>& item)
{
    playEquipSound(item);
    return mInventory->add(item);
}

double Player::getCurrentItemAttackStrengthDelay()
{
    return (float)(1.0 / getAttribute(SharedMonsterAttributes::ATTACK_SPEED)->getValue() * 20.0);
}

float Player::getSpeed()
{
    return static_cast<float>(getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());
}

float Player::getLuck()
{
    return getAttribute(SharedMonsterAttributes::LUCK)->getValue();
}

std::wstring Player::getName()
{
    return mName;
}

std::wstring Player::getNetworkName()
{
    return mName;
}

void Player::setName(const std::wstring& name)
{
    mName = name;
}

uint32_t Player::hash_fnct(boost::shared_ptr<Player> player)
{
    return static_cast<uint32_t>(boost::hash<std::wstring>()(player->mName));
}

bool Player::eq_test(boost::shared_ptr<Player> first, boost::shared_ptr<Player> second)
{
    return first->mName == second->mName;
}

bool Player::isImmobile()
{
    return getHealth() <= 0.0f || isSleeping();
}

void Player::CheckTimerAchievements()
{
    if (mLevel->mIsClientSide)
        return;
    if (!mTimerAchievementAwarded[eTimerAchievement_Castaway]
        && ++mTimerAchievementTicks[eTimerAchievement_Castaway] > 72000)
    {
        AwardTimerAchievement(eTimerAchievement_Castaway);
    }
    if (!mTimerAchievementAwarded[eTimerAchievement_SleepWithTheFishes]
        && ++mTimerAchievementTicks[eTimerAchievement_SleepWithTheFishes] > 24000)
    {
        AwardTimerAchievement(eTimerAchievement_SleepWithTheFishes);
    }
}

void Player::AwardTimerAchievement(TimerAchievements achievement)
{
    if (achievement == eTimerAchievement_Castaway)
    {
        if (!mLastAteKelp)
        {
            mTimerAchievementAwarded[achievement] = false;
        }
        else
        {
            awardStat(GenericStats::castaway(), GenericStats::param_castaway());
            mTimerAchievementAwarded[achievement] = true;
        }
    }
    else if (achievement == eTimerAchievement_SleepWithTheFishes)
    {
        awardStat(GenericStats::sleepWithTheFishes(), GenericStats::param_sleepWithTheFishes());
        mTimerAchievementAwarded[achievement] = true;
    }
    else
    {
        mTimerAchievementAwarded[achievement] = true;
    }
}

void Player::onChangedBlock(const BlockPos& position)
{
    if (!isSpectator())
        LivingEntity::onChangedBlock(position);
}

void Player::serverAiStep()
{
    LivingEntity::serverAiStep();
    updateSwingTime();
    mYHeadRot = mYRot;
}

bool Player::IsCreativeFlying()
{
    return mAbilities.mFlying;
}

uint8_t Player::IsSpectatingOtherPlayer()
{
    uint32_t spectating = 0;
    if ((mCamera || mPendingCameraId != -1) && isAlive())
    {
        spectating = 1;
    }
    return spectating;
}

void Player::SetCameraController(CameraController* controller, bool owned)
{
    if (mOwnsCameraController)
        delete mCameraController;
    mOwnsCameraController = owned;
    mCameraController = controller;
    if (controller != NULL)
        SetThirdPersonView(controller->getCameraMode());
}

void Player::SetCamera(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<Entity> self = shared_from_this();
    if (!entity || entity == self)
    {
        mCamera.reset();
    }
    else if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        mCamera = entity;
    }
    else
    {
        mCamera.reset();
    }
    mPendingCameraId = -1;
}

boost::shared_ptr<Entity> Player::GetCamera()
{
    return mCamera ? mCamera : shared_from_this();
}

uint8_t Player::SpectateAsOtherEntity()
{
    uint32_t spectating = 0;
    if (isAlive())
    {
        if (Minecraft::GetInstance()->GetSpectateMode() == eSpectateMode_Bat
            || Minecraft::GetInstance()->GetSpectateMode() == eSpectateMode_Parrot
            || Minecraft::GetInstance()->GetSpectateMode() == eSpectateMode_Vex)
            spectating = 1;
    }
    return spectating;
}

bool Player::SpectateAsFloatingHead()
{
    return isAlive() && Minecraft::GetInstance()->GetSpectateMode() == eSpectateMode_FloatingHead;
}

bool Player::SpectateAsInvisible()
{
    return isAlive() && Minecraft::GetInstance()->GetSpectateMode() == eSpectateMode_Invisible;
}

uint64_t Player::GetSpectateEntity()
{
    return Minecraft::GetInstance()->GetSpectateEntity();
}

void Player::StopSpectatingPlayer()
{
    Follow(false);
    SetCamera(boost::shared_ptr<Entity>());
}

void Player::pushEntities()
{
    if (!isSpectator())
        LivingEntity::pushEntities();
}

void Player::push(boost::shared_ptr<Entity> entity)
{
    if (!isSleeping())
        Entity::push(entity);
}

bool Player::canDestroy(const BlockState* state)
{
    return mInventory->canDestroy(state);
}

float Player::getDestroySpeed(const BlockState* state)
{
    float speed = mInventory->getDestroySpeed(state);
    if (speed > 1.0f)
    {
        int efficiencyLevel = EnchantmentHelper::getBlockEfficiency(
            boost::dynamic_pointer_cast<LivingEntity>(shared_from_this()));
        not_null_ptr<ItemInstance> item = getMainHandItem();
        if (efficiencyLevel > 0 && !item->isEmpty())
        {
            speed += static_cast<float>(efficiencyLevel * efficiencyLevel + 1);
        }
    }

    int digSpeedAmplifier = 0;
    if (hasEffect(MobEffects::DIG_SPEED))
    {
        digSpeedAmplifier = getEffect(MobEffects::DIG_SPEED)->getAmplifier();
    }
    if (hasEffect(MobEffects::CONDUIT_POWER))
    {
        int conduitAmplifier = getEffect(MobEffects::CONDUIT_POWER)->getAmplifier();
        if (conduitAmplifier + 1 > digSpeedAmplifier)
            digSpeedAmplifier = conduitAmplifier + 1;
    }
    if (digSpeedAmplifier > 0)
    {
        speed *= static_cast<float>(digSpeedAmplifier + 1) * 0.2f + 1.0f;
    }

    if (hasEffect(MobEffects::DIG_SLOWDOWN))
    {
        int amplifier = getEffect(MobEffects::DIG_SLOWDOWN)->getAmplifier();
        float multiplier;
        switch (amplifier)
        {
        case 0:
            multiplier = 0.3f;
            break;
        case 1:
            multiplier = 0.09f;
            break;
        case 2:
            multiplier = 0.0027f;
            break;
        default:
            multiplier = 0.00081f;
            break;
        }
        speed *= multiplier;
    }

    if (isUnderLiquid(Material::WATER)
        && !EnchantmentHelper::hasAquaAffinity(boost::dynamic_pointer_cast<LivingEntity>(shared_from_this())))
    {
        speed /= 5.0f;
    }
    return speed;
}

void Player::jumpFromGround()
{
    GameMode* gameMode = GetGameMode();
    if (gameMode != NULL)
        gameMode->RecordJump();
    LivingEntity::jumpFromGround();
    causeFoodExhaustion(
        static_cast<float>(isSprinting() ? FoodConstants::ExhaustionSprintJump() : FoodConstants::ExhaustionJump()));
}

void Player::awardStat(Stat*, arrayWithLength<unsigned char> parameters)
{
    if (parameters.data != NULL)
        delete[] parameters.data;
}

void Player::calculateFlapping()
{
    mOFlap = mFlap;
    mOFlapSpeed = mFlapSpeed;
    mFlapSpeed += (mOnGround ? -1 : 4) * 0.3f;
    mFlapSpeed = Mth::clamp(mFlapSpeed, 0.0f, 1.0f);
    if (!mOnGround && mField0x850 < 1.0f)
        mField0x850 = 1.0f;
    mField0x850 *= 0.9f;
    mFlap += mField0x850 * 2.0f;
}

uint32_t Player::CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID powerup)
{
    int32_t& ticks = mPowerupTicks[powerup];
    if (ticks <= 0)
        return 0;
    --ticks;
    return 1;
}

bool Player::canEat(bool alwaysEdible)
{
    return alwaysEdible || (mFoodData.needsFood() && !mAbilities.mInvulnerable && !hasInvulnerablePrivilege());
}

void Player::increaseXp(int32_t amount)
{
    increaseScore(amount);
    if (amount > INT_MAX - mTotalExperience)
        amount = INT_MAX - mTotalExperience;

    mExperienceProgress += static_cast<float>(amount) / getXpNeededForNextLevel();
    mTotalExperience += amount;
    while (mExperienceProgress >= 1.0f)
    {
        mExperienceProgress = (mExperienceProgress - 1.0f) * getXpNeededForNextLevel();
        giveExperienceLevels(1);
        mExperienceProgress /= getXpNeededForNextLevel();
    }
}

void Player::onEnchantmentPerformed(int32_t cost)
{
    if (!mAbilities.mInstabuild)
    {
        mExperienceLevel -= cost;
        if (mExperienceLevel < 0)
        {
            mExperienceLevel = 0;
            mTotalExperience = 0;
            mExperienceProgress = 0.0f;
        }
    }
    mEnchantmentSeed = mRandom->nextInt();
}

void Player::giveExperienceLevels(int32_t levels)
{
    mExperienceLevel += levels;
    if (mExperienceLevel < 0)
    {
        mExperienceLevel = 0;
        mTotalExperience = 0;
        mExperienceProgress = 0.0f;
    }

    if (levels > 0 && mExperienceLevel % 5 == 0 && (float)mLastLevelUpTime < (float)mTickCount - 100.0f)
    {
        float volume = mExperienceLevel > 30 ? 1.0f : static_cast<float>(mExperienceLevel) / 30.0f;
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::PLAYER_LEVELUP, getSoundSource(),
                          volume * 0.75f, 1.0f, 16.0f);
        mLastLevelUpTime = mTickCount;
    }
}

void Player::setXuid(PlayerUID xuid)
{
    mXuid = xuid;
    if ((xuid.flags & 0x20000000) != 0 || mIsGuest)
        setUUID(xuid.toString());
}

Scoreboard* Player::GetScoreboard()
{
    return mLevel->GetScoreboard();
}

Team* Player::getTeam()
{
    PlayerUID uid = getXuid() == PlayerUID(0) ? mOnlineXuid : mXuid;
    return GetScoreboard()->getPlayersTeam(uid);
}

float Player::getSleepRotation()
{
    if (mBedPosition == NULL)
        return 0.0f;
    const BlockState* state = mLevel->getBlockState(*mBedPosition);
    if (state->getBlock() != Blocks::bed)
        return 0.0f;
    int direction = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get3DDataValue();
    if (direction == 2)
        return 270.0f;
    if (direction == 3)
        return 90.0f;
    if (direction == 5)
        return 180.0f;
    return 0.0f;
}

const SoundEvent* Player::getFallDamageSound(int32_t damage)
{
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        return damage >= 2 ? SoundEvents::MG03_FALL_BIG : SoundEvents::MG03_FALL_SMALL;
    }
    return damage > 4 ? SoundEvents::PLAYER_BIG_FALL : SoundEvents::PLAYER_SMALL_FALL;
}

bool Player::canUnlock(LockCode* lock)
{
    if (lock->isEmpty())
        return true;
    not_null_ptr<ItemInstance> item = getMainHandItem();
    if (item->isEmpty() || item->hasCustomHoverName() == false)
        return false;
    return item->getHoverName() == lock->getKey();
}

bool Player::isInvisibleTo(boost::shared_ptr<Player> player)
{
    if (!isInvisible())
        return false;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (player->isSpectator() && !isSpectator() && !minecraft->GetClientMasterGameMode()->IsRoundEnding())
        return false;
    Team* team = getTeam();
    if (team != NULL && player && player->getTeam() == team && team->canSeeFriendlyInvisibles())
        return false;
    return true;
}

Vec3* Player::getAttachPos(Entity::EEntityLocation location, float partialTicks)
{
    Vec3* position = Entity::getAttachPos(location, partialTicks);
    if (location == Entity::ENTITY_LOCATION_BREATHING_POINT && isSwimming())
        position->y -= 1.100000023841858;
    return position;
}

uint8_t Player::CanRecordStats(Stat* stat, arrayWithLength<unsigned char> parameters)
{
    uint32_t canRecord = 0;
    GameMode* gameMode = GetGameMode();
    if (gameMode == NULL || gameMode->CanRecordStatsAndAchievements())
    {
        if (gApp.CanRecordStatsAndAchievements(stat, parameters) != 0 && stat != NULL)
            canRecord = 1;
    }
    return canRecord;
}

void Player::setAbsorptionAmount(float amount)
{
    if (amount < 0.0f)
        amount = 0.0f;
    getEntityData()->set(DATA_PLAYER_ABSORPTION_ID, amount, true);
}

float Player::getAbsorptionAmount()
{
    return getEntityData()->get(DATA_PLAYER_ABSORPTION_ID);
}

const HumanoidArm* Player::getMainArm()
{
    return mEntityData->get(DATA_MAIN_HAND) == 0 ? HumanoidArm::LEFT : HumanoidArm::RIGHT;
}

void Player::setMainArm(const HumanoidArm* arm)
{
    mEntityData->set(DATA_MAIN_HAND, static_cast<unsigned char>(arm == HumanoidArm::LEFT ? 0 : 1), true);
}

void Player::ResetTimerAchievement(TimerAchievements achievement)
{
    if (mLevel->mIsClientSide)
        return;
    mTimerAchievementTicks[achievement] = 0;
}

void Player::setRespawnPosition(BlockPos* position, bool forced)
{
    if (mRespawnPosition == position)
        return;
    delete mRespawnPosition;
    mRespawnPosition = position;
    mRespawnForced = position != NULL && forced;
}

void Player::closeContainer()
{
    mContainerMenu = mInventoryMenu;
}

CameraController* Player::GetCameraController()
{
    return mCameraController;
}

void Player::setBedOffset(const Direction* direction)
{
    mBedOffsetX = -1.8f * direction->getStepX();
    mBedOffsetZ = -1.8f * direction->getStepZ();
}

void Player::sendMessage(ClientboundChatPacket::EChatPacketMessage, int32_t*, uint32_t, std::wstring*, uint32_t)
{
}

bool Player::isHurt()
{
    return getHealth() > 0.0f && getHealth() < getMaxHealth();
}

int32_t Player::getExperienceReward(boost::shared_ptr<Player>)
{
    int32_t reward = 0;
    if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY))
    {
        reward = mExperienceLevel * 7;
        if (reward > 100)
            reward = 100;
    }
    return reward;
}

_SkinAdjustments Player::getSkinAdjustmentsById(uint32_t id)
{
    _SkinAdjustments adjustments;
    if ((id >> 31) != 0)
    {
        adjustments = gApp.GetSkinAdjustments(id);
        id &= 0x7fffffff;
        switch (id)
        {
        case 3:
        case 200:
        case 201:
        case 404:
        case 405:
        case 0x1f8:
        case 0x220:
        case 0x23a:
        case 0x23d:
        case 0x247:
            adjustments.mXMLVersion = 2;
            break;
        case 500:
            adjustments.mXMLVersion = 5;
            break;
        case 0x1fa:
            adjustments.mXMLVersion = 6;
            break;
        }
    }
    return adjustments;
}

std::wstring Player::getCapePathFromId(uint32_t id)
{
    wchar_t path[256];
    if ((id >> 31) != 0)
    {
        swprintf(path, 256, L"dlccape%08d.png", id & 0x7fffffff);
    }
    else if ((id & 0x7fffffe0) == 0)
    {
        swprintf(path, 256, L"defcape%08X.png", id & 0x1f);
    }
    else
    {
        swprintf(path, 256, L"ugccape%08X.png", id & 0x7fffffe0);
    }
    return path;
}

uint32_t Player::getCapeIdFromPath(const std::wstring& path)
{
    if (path.length() < 14)
        return 0;
    bool downloadable = path.compare(0, 3, L"dlc") == 0;
    std::wstring value = path.substr(7);
    std::wstring::size_type extension = value.find(L'.');
    if (extension != std::wstring::npos)
        value.erase(extension);

    uint32_t id = 0;
    std::wistringstream parser(value);
    if (downloadable)
        parser >> std::dec >> id;
    else
        parser >> std::hex >> id;
    return (id & 0x7fffffff) | (downloadable ? 0x80000000 : 0);
}

NonNullList<not_null_ptr<ItemInstance> > Player::getHandSlots()
{
    mField0x76c[0] = getMainHandItem();
    mField0x76c[1] = getOffhandItem();
    return mField0x76c;
}

NonNullList<not_null_ptr<ItemInstance> > Player::getArmorSlots()
{
    return mInventory->m_armor;
}

float Player::getEyeHeight()
{
    float height = 1.62f;
    if (isSleeping())
    {
        height = 0.2f;
    }
    else if (isSwimming())
    {
        height = 0.55f;
    }
    else if (getDamageNearbyMobs())
    {
        height = 0.4f;
    }
    else if (isSpectator())
    {
        if (SpectateAsOtherEntity())
        {
            height = (float)(mBoundingBox->mMaxY - mBoundingBox->mMinY) * 0.5f;
        }
        else if (SpectateAsFloatingHead())
        {
            height -= 1.5f;
        }
    }

    float eyeHeight = 0.4f;
    if (!isFallFlying() && (mBbHeight != 0.6f || isSwimming()))
    {
        eyeHeight = height;
        if (isSneaking() || mBbHeight == 1.65f)
        {
            eyeHeight = height - 0.08f;
        }
    }
    return eyeHeight;
}

bool Player::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    if (slot >= 0 && slot < static_cast<int>(mInventory->m_items.size()))
    {
        mInventory->setItem(slot, item);
        return 1;
    }
    const EquipmentSlot* armorSlot;
    if (slot == EquipmentSlot::HEAD->getIndex() + 100)
        armorSlot = EquipmentSlot::HEAD;
    else if (slot == EquipmentSlot::CHEST->getIndex() + 100)
        armorSlot = EquipmentSlot::CHEST;
    else if (slot == EquipmentSlot::LEGS->getIndex() + 100)
        armorSlot = EquipmentSlot::LEGS;
    else if (slot == EquipmentSlot::FEET->getIndex() + 100)
        armorSlot = EquipmentSlot::FEET;
    else
        armorSlot = NULL;

    if (slot == 98)
    {
        setItemSlot(EquipmentSlot::MAINHAND, item);
        return 1;
    }
    if (slot == 99)
    {
        setItemSlot(EquipmentSlot::OFFHAND, item);
        return 1;
    }
    if (armorSlot == NULL)
    {
        slot -= 200;
        if (slot < 0)
            return 0;
        if (slot >= static_cast<int>(mEnderChestInventory->getContainerSize()))
            return 0;
        mEnderChestInventory->setItem(slot, item);
        return 1;
    }
    if (!item->isEmpty() && item->getItem() != NULL)
    {
        const EquipmentSlot* naturalSlot;
        if (dynamic_cast<ArmorItem*>(item->getItem()) != NULL || dynamic_cast<ElytraItem*>(item->getItem()) != NULL)
            naturalSlot = Mob::getEquipmentSlotForItem(item);
        else
            naturalSlot = EquipmentSlot::HEAD;
        if (armorSlot != naturalSlot)
            return 0;
    }
    mInventory->setItem(armorSlot->getIndex() + mInventory->m_items.size(), item);
    OnEquipArmor(item);
    return 1;
}

uint32_t Player::getModelName()
{
    int skinId = static_cast<int>(CMinecraftApp::getSkinIdFromPath(mSkinLocation));
    if (skinId < 0 && gApp.IsFileInMemoryTextures(mSkinLocation))
        return GetModelTypeFromAnimBitmask(gApp.GetSkinAdjustments(skinId).mAnimFlags);
    return GetModelTypeFromTextureId(getTexture());
}

void Player::setCustomSkin(uint32_t skin)
{
    setIsIdle(false);
    setSkinAdjustments(getSkinAdjustmentsById(skin));
    uint32_t defaultSkin = mPlayerIndex + 1;
    if ((skin >> 31) == 0)
    {
        if ((skin & 0x7fffffe0) == 0)
        {
            defaultSkin = skin & 0x1f;
            if (defaultSkin == 0)
                defaultSkin = mPlayerIndex + 1;
        }
    }
    setPlayerDefaultSkin(static_cast<EDefaultSkins>(defaultSkin));
    mCustomSkin = skin;
    mSkinLocation = CMinecraftApp::getSkinPathFromId(skin);
    mAdditionalModelPartsChecked = 0;
    mDLCSkinChecked = 0;
    SetAdditionalModelParts(NULL);
}

uint64_t Player::setCustomCape(uint32_t cape)
{
    mCustomCape = cape;
    if (cape == 0)
    {
        MOJANG_DATA* data = gApp.GetMojangDataForXuid(mOnlineXuid);
        if (data == NULL)
        {
            if (gApp.DefaultCapeExists())
                mCapeLocation = L"Special_Cape.png";
            else
                mCapeLocation = L"";
        }
        else
        {
            if (data->capePath[0] != 0)
            {
                mCapeLocation = data->capePath;
                return cape;
            }
            if (gApp.DefaultCapeExists())
                mCapeLocation = L"Special_Cape.png";
            else
                mCapeLocation = L"";
        }
    }
    else
    {
        mCapeLocation = getCapePathFromId(cape);
    }
    return cape;
}

std::vector<ModelPart*>* Player::GetAdditionalModelParts()
{
    if (mAdditionalModelParts != NULL || mAdditionalModelPartsChecked)
        return mAdditionalModelParts;

    bool defaultSkin = mSkinLocation.substr(0, 3).compare(L"def") == 0;
    mAdditionalModelParts = gApp.GetAdditionalModelParts(static_cast<unsigned int>(mCustomSkin));
    if (mSkinLocation.empty() || defaultSkin || mAdditionalModelParts != NULL
        || gApp.IsFileInMemoryTextures(mSkinLocation))
    {
        mAdditionalModelPartsChecked = true;
    }
    if (mAdditionalModelParts == NULL && !mDLCSkinChecked)
    {
        mDLCSkinChecked = true;
        DLCSkinFile* skinFile = static_cast<DLCSkinFile*>(gApp.mDLCManager.getSkinFile(mSkinLocation));
        if (skinFile != NULL)
        {
            if (skinFile->getAdditionalBoxesCount() != 0)
                mAdditionalModelParts = gApp.SetAdditionalSkinBoxes(mCustomSkin, skinFile->getAdditionalBoxes(),
                                                                    skinFile->getSkinAdjustments());
            gApp.SetSkinAdjustments(skinFile->m_fileID, skinFile->getSkinAdjustments());
            mAdditionalModelPartsChecked = true;
        }
    }

    if (mAdditionalModelPartsChecked)
        setSkinAdjustments(getSkinAdjustmentsById(mCustomSkin));
    return mAdditionalModelParts;
}

void Player::tickArmor()
{
    not_null_ptr<ItemInstance> helmet = getItemSlot(EquipmentSlot::HEAD);
    if (helmet->getItem() == Items::turtleHelmet && !headInWater())
    {
        addEffect(new MobEffectInstance(MobEffects::WATER_BREATHING, 200, 0, false, false),
                  boost::shared_ptr<Entity>());
    }
}

void Player::updateFrameTick()
{
    if (!isSpectator())
    {
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true) && !mAbilities.mFlying)
        {
            if (!mOnGround)
            {
                if (!mPlayedFallingSound)
                {
                    CommonMasterGameMode* gameMode = NULL;
                    if (isLocalPlayer())
                        gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
                    else if (gMinecraftServer != NULL)
                        gameMode = gMinecraftServer->mMasterGameMode;
                    if (gameMode != NULL && gameMode->IsRoundPlaying() && gameMode->HasFallenOffLayer(mLastLayerY, mY))
                    {
                        mPlayedFallingSound = true;
                        playSound(SoundEvents::MG02_FALLING, 1.0f, 1.0f);
                    }
                }
            }
            else if (mPlayedFallingSound)
            {
                mPlayedFallingSound = false;
            }
            mLastLayerY = mY;
        }
    }
    else
    {
        mOnGround = false;
    }

    if (mTakeXpDelay > 0)
        --mTakeXpDelay;

    if (isSleeping())
    {
        mSleepCounter = std::min(mSleepCounter + 1, 100);
        if (!mLevel->mIsClientSide)
        {
            if (!checkBedExists())
                stopSleepInBed(true, true, false);
            else if (mLevel->isDay())
                stopSleepInBed(false, true, true);
        }
    }
    else if (mSleepCounter > 0)
    {
        if (++mSleepCounter >= 110)
            mSleepCounter = 0;
    }

    if (!isAlive())
        mDeathFadeTimer = std::min(mDeathFadeTimer + 1, 21);
}

void Player::tickCamera()
{
    if (mPendingCameraId != -1)
    {
        boost::shared_ptr<Entity> camera = mLevel->getEntity(mPendingCameraId);
        if (camera != NULL)
        {
            mPendingCameraId = -1;
            ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
            if (isSpectator() || gameMode->IsRoundEnding())
            {
                SetCamera(camera);
            }
        }
    }

    if (!isSpectator() && mCamera != NULL)
    {
        StopSpectatingPlayer();
    }

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && mCamera != NULL)
    {
        mXRot += Mth::clamp(Mth::wrapDegrees(mCamera->mXRot - mXRot), -2.25f, 2.25f);
        mYRot += Mth::clamp(Mth::wrapDegrees(mCamera->mYRot - mYRot), -2.25f, 2.25f);
    }
}

void Player::PlayerRingSoundEffect(int ring)
{
    if (mLevel->mIsClientSide)
    {
        bool localPlayer = isLocalPlayer();
        SoundEngine* soundEngine = Minecraft::GetInstance()->mSoundEngine;
        if (ring < 7)
        {
            if (ring < 5)
            {
                if (!mRingSoundAlternate)
                {
                    if (!localPlayer)
                        soundEngine->play(SoundEvents::MG03_EMERALD_POINT_TICK, static_cast<float>(mX),
                                          static_cast<float>(mY), static_cast<float>(mZ), 1.0f, 1.0f, NULL);
                    else
                        soundEngine->playUI(SoundEvents::MG03_EMERALD_POINT_TICK, 1.0f, 1.0f);
                    mRingSoundAlternate ^= 1;
                }
                else
                {
                    if (!localPlayer)
                        soundEngine->play(SoundEvents::MG03_EMERALD_POINT_TOK, static_cast<float>(mX),
                                          static_cast<float>(mY), static_cast<float>(mZ), 1.0f, 1.0f, NULL);
                    else
                        soundEngine->playUI(SoundEvents::MG03_EMERALD_POINT_TOK, 1.0f, 1.0f);
                    mRingSoundAlternate ^= 1;
                }
            }
            else
            {
                if (!mRingSoundAlternate)
                {
                    if (!localPlayer)
                        soundEngine->play(SoundEvents::MG03_GOLD_POINT_TICK, static_cast<float>(mX),
                                          static_cast<float>(mY), static_cast<float>(mZ), 1.0f, 1.0f, NULL);
                    else
                        soundEngine->playUI(SoundEvents::MG03_GOLD_POINT_TICK, 1.0f, 1.0f);
                    mRingSoundAlternate ^= 1;
                }
                else
                {
                    if (!localPlayer)
                        soundEngine->play(SoundEvents::MG03_GOLD_POINT_TOK, static_cast<float>(mX),
                                          static_cast<float>(mY), static_cast<float>(mZ), 1.0f, 1.0f, NULL);
                    else
                        soundEngine->playUI(SoundEvents::MG03_GOLD_POINT_TOK, 1.0f, 1.0f);
                    mRingSoundAlternate ^= 1;
                }
            }
        }
        else
        {
            if (!localPlayer)
                soundEngine->play(SoundEvents::MG03_DIAMOND_POINT, static_cast<float>(mX), static_cast<float>(mY),
                                  static_cast<float>(mZ), 1.0f, 1.0f, NULL);
            else
                soundEngine->playUI(SoundEvents::MG03_DIAMOND_POINT, 1.0f, 1.0f);
        }
    }
}

void Player::prepareCustomTextures()
{
    MOJANG_DATA* data = gApp.GetMojangDataForXuid(mOnlineXuid);
    if (data != NULL && data->skinPath[0] != L'\0')
    {
        mSkinLocation.assign(data->skinPath, std::wcslen(data->skinPath));
    }
}

void Player::_init()
{
    mInventory = boost::shared_ptr<Inventory>(new Inventory(this));
    mPrevCameraYaw = 0.0f;
    mCameraYaw = 0.0f;
    mTakeXpDelay = 0;
    mSkinLocation = L"";
    mCapeLocation = L"";
    mField0x66c = 0;
    mPrevChasingPosX = 0.0;
    mPrevChasingPosY = 0.0;
    mPrevChasingPosZ = 0.0;
    mChasingPosX = 0.0;
    mChasingPosY = 0.0;
    mChasingPosZ = 0.0;
    mSleeping = 0;
    mBedPosition = NULL;
    mSleepCounter = 0;
    mDeathFadeTimer = 0;
    mBedOffsetX = 0.0f;
    mBedOffsetY = 0.0f;
    mBedOffsetZ = 0.0f;
    mField0x6dc = 0;
    mRespawnPosition = NULL;
    mRespawnForced = 0;
    mStartRidingPos = NULL;
    mWalkDistanceCm = 0;
    mSwimDistanceCm = 0;
    mDiveDistanceCm = 0;
    mFallDistanceCm = 0;
    mClimbDistanceCm = 0;
    mMinecartDistanceCm = 0;
    mBoatDistanceCm = 0;
    mPigDistanceCm = 0;
    mJumpTriggerTime = 0;
    mExperienceLevel = 0;
    mTotalExperience = 0;
    mExperienceProgress = 0.0f;
    mFishing.reset();
    mDefaultFlySpeed = 0.02f;
    mField0x738 = NULL;
    mLastLevelUpTime = 0;
    mName = L"";
    mReducedDebugInfo = 0;
    mEnderChestInventory.reset(new PlayerEnderChestContainer());
    mField0x7c8 = NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY);
    mField0x76c = NonNullList<not_null_ptr<ItemInstance> >::withSize(2, ItemInstance::EMPTY);
    mField0x764 = ItemInstance::EMPTY;
    mCameraController = NULL;
    mOwnsCameraController = true;
    mWinnerParticleTicks = 0;
    mField0x781 = 0;
    mWasFallFlying = false;
    mDebugOptions = 0;
    mGamePrivileges = 0;
    mAdditionalModelParts = NULL;
    mAdditionalModelPartsChecked = 0;
    mDLCSkinChecked = 0;
    mPendingCameraId = -1;
    mCamera.reset();
    mPositionLocked = false;
    mField0x815 = false;
    mField0x818 = 1.0;
    mSpectatorSkullEquipped = 0;
    mField0x828 = 0;
    mRingSoundAlternate = 0;
    mFlap = 0.0f;
    mFlapSpeed = 0.0f;
    mOFlapSpeed = 0.0f;
    mOFlap = 0.0f;
    mField0x850 = 1.0f;
    mField0x854 = 15.0f;
    mField0x858 = 0.0f;
    mLastAteKelp = false;
    mTimerAchievementAwarded[0] = false;
    mTimerAchievementAwarded[1] = false;
    mTimerAchievementTicks[0] = 0;
    mTimerAchievementTicks[1] = 0;
    mCategories |= EntityCategory::eEntityCategory_Player;
    mInsomniaComponent = new InsomniaComponent(this, 3.0f);
}

void Player::tick()
{
    if (!gApp.IsAppPaused())
        CheckTimerAchievements();

    mWasFallFlying = isFallFlying();
    isSpectator();
    if (mLevel->mIsClientSide)
        updateFrameTick();

    if (isOnFire() && (mAbilities.mInvulnerable || hasInvulnerablePrivilege()))
        clearFire();

    PIXBeginNamedEvent(0.0f, "LivingEntity::tick");
    LivingEntity::tick();
    PIXEndNamedEvent();
    tickCamera();

    if (!mLevel->mIsClientSide && mContainerMenu != NULL
        && !mContainerMenu->stillValid(boost::static_pointer_cast<Player>(shared_from_this())))
    {
        closeContainer();
        mContainerMenu = mInventoryMenu;
    }

    moveCloak();
    if (!isPassenger())
    {
        delete mStartRidingPos;
        mStartRidingPos = NULL;
    }

    if (!mLevel->mIsClientSide)
    {
        PIXBeginNamedEvent(0.0f, "Tick food data");
        mFoodData.tick(boost::static_pointer_cast<Player>(shared_from_this()));
        PIXEndNamedEvent();
    }

    double clampedX = mX;
    double clampedY = mY;
    double clampedZ = mZ;
    mLevel->clampWithinLevelBounds(clampedX, clampedZ, 0.0);
    mLevel->clampWithinLevelHeight(clampedY, static_cast<double>(mBbHeight));
    if (clampedX != mX || clampedZ != mZ || clampedY != mY)
        setPos(clampedX, clampedY, clampedZ);

    ++mAttackStrengthTicker;
    not_null_ptr<ItemInstance> heldItem = getMainHandItem();
    if (!ItemInstance::matches(mField0x764, heldItem, true))
    {
        if (!ItemInstance::isSameIgnoreDurability(mField0x764, heldItem))
            resetAttackStrengthTicker();
        mField0x764 = heldItem->isEmpty() ? ItemInstance::EMPTY : heldItem->copy();
    }

    tickArmor();
    mField0x738->tick();
    updatePlayerSize();

    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!mLevel->mIsClientSide || !Minecraft::InMiniGame(eMiniGameId_Any, true)
        || Minecraft::InMiniGame(eMiniGameId_Lobby, true))
    {
        return;
    }

    ClientScoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
    if (scoreboard->IsWinner(getSUID(), -1) && !Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        for (int i = 0; i < 2; ++i)
        {
            ++mWinnerParticleTicks;
            float red = Mth::sin(static_cast<float>(mWinnerParticleTicks));
            float green = Mth::sin(static_cast<float>(mWinnerParticleTicks) + 2.0f);
            float blue = Mth::sin(static_cast<float>(mWinnerParticleTicks) + 4.0f);
            double y = mRandom->nextDouble() * mBbHeight + mY;
            mLevel->addParticle(PARTICLE_MOB_SPELL, mX, y, mZ, (red * 127.0f + 128.0f) / 255.0f,
                                (green * 127.0f + 128.0f) / 255.0f, (blue * 127.0f + 128.0f) / 255.0f,
                                arrayWithLength<int>());
        }
        return;
    }

    mWinnerParticleTicks = 0;
    if (getSharedFlag(FLAG_INVULNERABLE) && Minecraft::InMiniGame(eMiniGameId_Tumble, true))
    {
        if (scoreboard->IsWinningOnMatchPoint(getSUID()))
        {
            unsigned int colour = PlayerRenderer::getNametagColour(mPlayerIndex);
            if (mPlayerIndex == 0)
                colour = 0xffffffff;
            arrayWithLength<int> colourData(4, true);
            colourData.data[0] = (colour >> 16) & 0xff;
            colourData.data[1] = (colour >> 8) & 0xff;
            colourData.data[3] = 0xbf;
            colourData.data[2] = colour & 0xff;
            float angle = (mYBodyRot + 90.0f) * Mth::DEG_TO_RAD;
            Vec3* direction = Vec3::newTemp(cosf(angle), 0.0, sinf(angle));
            double y = mRandom->nextDouble() * mBbHeight + mY;
            mLevel->addParticle(PARTICLE_WINNING_INSTANT_SPELL, mX - direction->x * 0.1f, y, mZ - direction->z * 0.1f,
                                -(direction->x * 0.1f), 0.0, -(direction->z * 0.1f), colourData);
        }
    }
    else if (Minecraft::InMiniGame(eMiniGameId_Glide, true)
             && Minecraft::GetInstance()->GetClientMasterGameMode()->IsInActiveRoundState())
    {
        scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
        int points = scoreboard->GetPoints(getSUID(), -1);
        if (mLastPoints != points)
        {
            int gained = points - mLastPoints;
            mLastPoints = points;
            if (points != 0 && gained > 0)
            {
                eMinecraftColour colourId = eMinecraftColour_Particle_ScoreRing_Small;
                if (gained < 7)
                    colourId = static_cast<eMinecraftColour>(gained > 4 ? 0x1d4 : 0x1d3);
                PlayerRingSoundEffect(gained);
                unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(colourId);
                unsigned int red = (colour >> 16) & 0xff;
                unsigned int green = (colour >> 8) & 0xff;
                unsigned int blue = colour & 0xff;
                for (int i = 0; i < 150; ++i)
                {
                    arrayWithLength<int> colourData(4, true);
                    colourData.data[0] = red;
                    colourData.data[2] = blue;
                    colourData.data[1] = green;
                    colourData.data[3] = 0xff;
                    double xo = mRandom->nextDouble() * 2.0 - 1.0;
                    double yo = mBbHeight * (mRandom->nextDouble() * 2.0 - 1.0);
                    double zo = mRandom->nextDouble() * 2.0 - 1.0;
                    mLevel->addParticle(PARTICLE_FIREWORKS_SPARK, mX + xo, mY + yo, mZ + zo, mXd + xo, mYd + yo,
                                        mZd + zo, colourData);
                }
            }
        }

        int checkpointId = scoreboard->GetLastCheckpointID(getSUID(), -1);
        if (checkpointId != -1 && mLastCheckpointId != checkpointId)
        {
            CheckpointRuleDefinition* checkpoint =
                gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpointByID(checkpointId);
            if (checkpoint->hasSpawnPoint())
            {
                mLastCheckpointId = checkpointId;
                if (mLevel->mIsClientSide)
                {
                    SoundEngine* soundEngine = Minecraft::GetInstance()->mSoundEngine;
                    if (!isLocalPlayer())
                    {
                        soundEngine->play(SoundEvents::MG03_CHECKPOINT, static_cast<float>(mX), static_cast<float>(mY),
                                          static_cast<float>(mZ), 1.0f, 1.0f, NULL);
                    }
                    else
                    {
                        soundEngine->playUI(SoundEvents::MG03_CHECKPOINT, 1.0f, 1.0f);
                    }
                }
            }
        }

        if (!mEndZoneEffectShown && scoreboard->GetEndZoneEntered(getSUID(), -1))
        {
            mEndZoneEffectShown = true;
            for (int i = 0; i < 150; ++i)
            {
                ++mWinnerParticleTicks;
                float red = Mth::sin(static_cast<float>(mWinnerParticleTicks));
                float green = Mth::sin(static_cast<float>(mWinnerParticleTicks) + 2.0f);
                float blue = Mth::sin(static_cast<float>(mWinnerParticleTicks) + 4.0f);
                arrayWithLength<int> colourData(4, true);
                colourData.data[2] = static_cast<int>(blue * 127.0f + 128.0f);
                colourData.data[0] = static_cast<int>(red * 127.0f + 128.0f);
                colourData.data[1] = static_cast<int>(green * 127.0f + 128.0f);
                colourData.data[3] = 0xff;
                double xo = mRandom->nextDouble() * 2.0 - 1.0;
                double yo = mBbHeight * (mRandom->nextDouble() * 2.0 - 1.0);
                double zo = mRandom->nextDouble() * 2.0 - 1.0;
                mLevel->addParticle(PARTICLE_FIREWORKS_SPARK, mX + xo, mY + yo, mZ + zo, mXd + xo, mYd + yo, mZd + zo,
                                    colourData);
            }
        }
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && miniGame->ShowInvulnerableEffect()
        && GetGameType() != GameType::SPECTATOR && getSharedFlag(FLAG_INVULNERABLE))
    {
        unsigned int colour = PlayerRenderer::getNametagColour(mPlayerIndex);
        if (mPlayerIndex == 0)
            colour = 0xffffffff;
        double y = mRandom->nextDouble() * mBbHeight + mY;
        mLevel->addParticle(PARTICLE_MOB_SPELL, mX, y, mZ, static_cast<float>((colour >> 16) & 0xff) / 255.0f,
                            static_cast<float>((colour >> 8) & 0xff) / 255.0f,
                            static_cast<float>(colour & 0xff) / 255.0f, arrayWithLength<int>());
    }
}

void Player::rideTick()
{
    if (!mLevel->mIsClientSide && isSneaking() && isPassenger())
    {
        stopRiding();
        setSneaking(false);
        return;
    }

    double x = mX;
    double y = mY;
    double z = mZ;
    LivingEntity::rideTick();
    mPrevCameraYaw = mCameraYaw;
    mCameraYaw = 0.0f;
    checkRidingStatistiscs(mX - x, mY - y, mZ - z);

    if (isPassenger() && getVehicle()->GetType() == EntityType::PIG)
    {
        boost::shared_ptr<Pig> pig = boost::static_pointer_cast<Pig>(getVehicle());
        float bodyRot = pig->mYBodyRot;
        mYBodyRot = bodyRot;
        while (bodyRot - mYBodyRotO < -180.0f)
            mYBodyRotO -= 360.0f;
        while (bodyRot - mYBodyRotO >= 180.0f)
            mYBodyRotO += 360.0f;
    }
    else if (isPassenger() && getVehicle()->GetType() == EntityType::HORSE)
    {
        boost::shared_ptr<Horse> horse = boost::static_pointer_cast<Horse>(getVehicle());
        float bodyRot = horse->mYBodyRot;
        mYBodyRot = bodyRot;
        while (bodyRot - mYBodyRotO < -180.0f)
            mYBodyRotO -= 360.0f;
        while (bodyRot - mYBodyRotO >= 180.0f)
            mYBodyRotO += 360.0f;
    }
}

void Player::aiStep()
{
    if (mJumpTriggerTime > 0)
        --mJumpTriggerTime;

    if (mLevel->getDifficulty() == Difficulty::PEACEFUL
        && mLevel->getGameRules()->getBoolean(GameRules::RULE_NATURAL_REGENERATION))
    {
        if (getHealth() < getMaxHealth() && mTickCount % 20 * 12 == 0)
            heal(1.0f);
        if (mFoodData.needsFood() && mTickCount % 10 == 0)
            mFoodData.setFoodLevel(mFoodData.getFoodLevel() + 1);
    }

    PIXBeginNamedEvent(0.0f, "Inventory tick");
    mInventory->tick();
    PIXEndNamedEvent();

    mPrevCameraYaw = mCameraYaw;

    PIXBeginNamedEvent(0.0f, "LivingEntity::aiStep");
    LivingEntity::aiStep();
    PIXEndNamedEvent();

    AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    if (!mLevel->mIsClientSide)
        speed->setBaseValue(mAbilities.getWalkingSpeed());

    mFlyingSpeed = mDefaultFlySpeed;
    if (isSprinting())
        mFlyingSpeed += mDefaultFlySpeed * 0.3f;

    setSpeed(static_cast<float>(speed->getValue()));

    const double horizontalSpeedSq = mXd * mXd + mZd * mZd;
    float horizontalSpeed = static_cast<float>(std::sqrt(horizontalSpeedSq));
    if (horizontalSpeedSq < 9.999999747378752e-06)
        horizontalSpeed = 0.0f;
    float verticalAngle = static_cast<float>(std::atan(-mYd * 0.20000000298023224)) * 15.0f;
    if (horizontalSpeed > 0.1f)
        horizontalSpeed = 0.1f;
    if (!mOnGround || getHealth() <= 0.0f)
        horizontalSpeed = 0.0f;
    if (mOnGround || getHealth() <= 0.0f)
        verticalAngle = 0.0f;
    mCameraYaw += (horizontalSpeed - mCameraYaw) * 0.4f;
    mBob += (verticalAngle - mBob) * 0.8f;

    if (getHealth() > 0.0f && !isSpectator())
    {
        PIXBeginNamedEvent(0.0f, "Touch nearby entities");
        AABB* touchBox;
        if (isPassenger() && !getVehicle()->mRemoved)
            touchBox = mBoundingBox->minmax(getVehicle()->mBoundingBox)->grow(1.0, 0.0, 1.0);
        else
            touchBox = mBoundingBox->grow(1.0, 0.5, 1.0);

        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(shared_from_this(), touchBox, NULL);
        if (entities != NULL)
        {
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
            {
                if (!(*it)->mRemoved)
                    touch(*it);
            }
        }
        PIXEndNamedEvent();
    }

    playShoulderEntityAmbientSound(getShoulderEntityLeft());
    playShoulderEntityAmbientSound(getShoulderEntityRight());
    if (!mLevel->mIsClientSide && (mFallDistance > 0.5f || isInWater() || isPassenger() || mAbilities.mFlying))
    {
        removeEntitiesOnShoulder();
    }

    calculateFlapping();
}

void Player::touch(boost::shared_ptr<Entity> entity)
{
    entity->playerTouch(boost::static_pointer_cast<Player>(shared_from_this()));
}

void Player::destroyVanishingCursedItems()
{
    for (unsigned int i = 0; i < mInventory->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = mInventory->getItem(i);
        if (!item->isEmpty() && EnchantmentHelper::hasVanishingCurse(item))
            mInventory->removeItemNoUpdate(i);
    }
}

void Player::addWalkers(DataFixerUpper* fixer)
{
    std::vector<std::wstring>* itemFields = new std::vector<std::wstring>();
    itemFields->push_back(L"Inventory");
    itemFields->push_back(L"EnderItems");
    fixer->addWalker(DataFixTypes::eDataFixTypes_Player, new PlayerDataWalker(itemFields));
}

void Player::readAdditionalSaveData(CompoundTag* tag)
{
    LivingEntity::readAdditionalSaveData(tag);

    mInventory->load(tag->getList(L"Inventory", Tag::TAG_COMPOUND));
    mInventory->m_selected = tag->getInt(L"SelectedItemSlot");
#ifdef MINECRAFT_PC
    PcModStorageRead(tag, &mModData);
#endif

    mSleeping = tag->getBoolean(L"Sleeping");
    mSleepCounter = tag->getShort(L"SleepTimer");

    mExperienceProgress = tag->getFloat(L"XpP");
    mExperienceLevel = tag->getInt(L"XpLevel");
    mTotalExperience = tag->getInt(L"XpTotal");
    mEnchantmentSeed = tag->getInt(L"XpSeed");
    if (mEnchantmentSeed == 0)
        mEnchantmentSeed = mRandom->nextInt();

    setScore(tag->getInt(L"Score"));

    if (mSleeping != 0)
    {
        if (mBedPosition == NULL)
            mBedPosition = new BlockPos();
        *mBedPosition = BlockPos(shared_from_this());
        stopSleepInBed(true, true, false);
    }

    if (tag->contains(L"SpawnX", Tag::TAG_ANY_NUMERIC) && tag->contains(L"SpawnY", Tag::TAG_ANY_NUMERIC)
        && tag->contains(L"SpawnZ", Tag::TAG_ANY_NUMERIC))
    {
        if (mRespawnPosition == NULL)
            mRespawnPosition = new BlockPos();
        *mRespawnPosition = BlockPos(tag->getInt(L"SpawnX"), tag->getInt(L"SpawnY"), tag->getInt(L"SpawnZ"));
        mRespawnForced = tag->getBoolean(L"SpawnForced");
    }

    mFoodData.readAdditionalSaveData(tag);
    mAbilities.loadSaveData(tag);

    if (tag->contains(L"EnderItems", Tag::TAG_LIST))
    {
        mEnderChestInventory->setItemsByTag(tag->getList(L"EnderItems", Tag::TAG_COMPOUND));
    }

    if (tag->contains(L"ShoulderEntityLeft", Tag::TAG_COMPOUND))
        setShoulderEntityLeft(static_cast<CompoundTag*>(tag->getCompound(L"ShoulderEntityLeft")->copy()));
    if (tag->contains(L"ShoulderEntityRight", Tag::TAG_COMPOUND))
        setShoulderEntityRight(static_cast<CompoundTag*>(tag->getCompound(L"ShoulderEntityRight")->copy()));

    mGamePrivileges = tag->getInt(L"GamePrivileges");
}

void Player::addAdditonalSaveData(CompoundTag* tag)
{
    LivingEntity::addAdditonalSaveData(tag);
    tag->putInt(SharedConstants::DATA_VERSION_TAG, 922);

    tag->put(L"Inventory", mInventory->save(new ListTag()));
    tag->putInt(L"SelectedItemSlot", mInventory->m_selected);
#ifdef MINECRAFT_PC
    PcModStorageWrite(tag, mModData);
#endif

    tag->putBoolean(L"Sleeping", mSleeping != 0);
    tag->putShort(L"SleepTimer", static_cast<short>(mSleepCounter));

    tag->putFloat(L"XpP", mExperienceProgress);
    tag->putInt(L"XpLevel", mExperienceLevel);
    tag->putInt(L"XpTotal", mTotalExperience);
    tag->putInt(L"XpSeed", mEnchantmentSeed);
    tag->putInt(L"Score", static_cast<int>(getScore()));

    if (mRespawnPosition != NULL)
    {
        tag->putInt(L"SpawnX", mRespawnPosition->x);
        tag->putInt(L"SpawnY", mRespawnPosition->y);
        tag->putInt(L"SpawnZ", mRespawnPosition->z);
        tag->putBoolean(L"SpawnForced", mRespawnForced != 0);
    }

    mFoodData.addAdditonalSaveData(tag);
    mAbilities.addSaveData(tag);

    tag->put(L"EnderItems", mEnderChestInventory->createTag());

    if (!getShoulderEntityLeft()->isEmpty())
        tag->put(L"ShoulderEntityLeft", getShoulderEntityLeft()->copy());
    if (!getShoulderEntityRight()->isEmpty())
        tag->put(L"ShoulderEntityRight", getShoulderEntityRight()->copy());

    not_null_ptr<ItemInstance> selected = mInventory->getSelected();
    if (!selected->isEmpty() && selected->getItem() != NULL)
    {
        tag->put(L"SelectedItem", selected->save(new CompoundTag()));
    }

    tag->putInt(L"GamePrivileges", mGamePrivileges);
}

boost::shared_ptr<ItemEntity> Player::drop(not_null_ptr<ItemInstance> item, bool randomThrow, bool retainOwnership)
{
    if (item->isEmpty())
        return boost::shared_ptr<ItemEntity>();

    double dropY = mY - 0.3f + getEyeHeight();
    boost::shared_ptr<ItemEntity> dropped(new ItemEntity(mLevel, mX, dropY, mZ, item));
    dropped->setPickUpDelay(40);
    if (retainOwnership)
    {
        dropped->setThrower(getName());
    }

    if (randomThrow)
    {
        float speed = mRandom->nextFloat() * 0.5f;
        float angle = mRandom->nextFloat() * 6.2831855f;
        dropped->mXd = -static_cast<double>(sinf(angle) * speed);
        dropped->mZd = static_cast<double>(cosf(angle) * speed);
        dropped->mYd = 0.2f;
    }
    else
    {
        float power = 0.3f;
        dropped->mXd = -(sinf(mYRot * Mth::DEG_TO_RAD) * cosf(mXRot * Mth::DEG_TO_RAD) * power);
        dropped->mZd = cosf(mYRot * Mth::DEG_TO_RAD) * cosf(mXRot * Mth::DEG_TO_RAD) * power;
        dropped->mYd = 0.1f - sinf(mXRot * Mth::DEG_TO_RAD) * power;

        float randAngle = mRandom->nextFloat() * 6.2831855f;
        float randRadius = 0.02f * mRandom->nextFloat();
        dropped->mXd += cosf(randAngle) * randRadius;
        dropped->mYd += (mRandom->nextFloat() - mRandom->nextFloat()) * 0.1f;
        dropped->mZd += sinf(randAngle) * randRadius;
    }

    not_null_ptr<ItemInstance> droppedItem = reallyDrop(dropped);
    return dropped;
}

boost::shared_ptr<ItemEntity> Player::drop(not_null_ptr<ItemInstance> item, bool retainOwnership)
{
    return drop(item, false, retainOwnership);
}

boost::shared_ptr<ItemEntity> Player::drop(bool dropAll)
{
    int count = dropAll && !mInventory->getSelected()->isEmpty() ? mInventory->getSelected()->getCount() : 1;
    not_null_ptr<ItemInstance> dropped = mInventory->removeItem(mInventory->m_selected, count);
    if (mInventory->m_items[mInventory->m_selected]->isEmpty())
    {
        mInventory->OnRemove(mInventory->m_selected);
    }
    return drop(dropped, true);
}

not_null_ptr<ItemInstance> Player::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    mLevel->addEntity(item);
    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::ITEM_COLLECT,
                      SoundSource::eSoundSource_Players, 0.3f, 1.0f, 0.0f);
    return item->getItem();
}

void Player::die(const DamageSource* source)
{
    LivingEntity::die(source);
    setSize(0.2f, 0.2f);
    setPos(mX, mY, mZ);
    mYd = 0.10000000149011612;
    if (gApp.isXuidNotch(mXuid))
        drop(not_null_ptr<ItemInstance>(new ItemInstance(Items::apple, 1)), true, false);
    if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY) && !isSpectator())
    {
        destroyVanishingCursedItems();
        mInventory->dropAll();
    }
    if (source != NULL)
    {
        mXd = -Mth::cos((mHurtDir + mYRot) * Mth::DEG_TO_RAD) * 0.1f;
        mZd = -Mth::sin((mHurtDir + mYRot) * Mth::DEG_TO_RAD) * 0.1f;
    }
    else
    {
        mXd = 0.0;
        mZd = 0.0;
    }
    clearFire();
    setSharedFlag(FLAG_ONFIRE, false);
}

const SoundEvent* Player::getHurtSound(const DamageSource* source)
{
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        return SoundEvents::MG03_HIT;
    if (source == DamageSource::ON_FIRE)
        return SoundEvents::PLAYER_HURT_ON_FIRE;
    if (source == DamageSource::DROWN)
        return SoundEvents::PLAYER_HURT_DROWN;
    return SoundEvents::PLAYER_HURT;
}

bool Player::hurt(const DamageSource* source, float damage)
{
    {
        boost::shared_ptr<Entity> directEntity = source->getDirectEntity();
        if (damage == 0.0f && Minecraft::InMiniGame(eMiniGameId_Any, true) && directEntity)
        {
            if (eTYPE_DERIVED_FROM(EntityType::FISHING_HOOK, directEntity->GetType())
                || eTYPE_DERIVED_FROM(EntityType::SNOWBALL, directEntity->GetType())
                || eTYPE_DERIVED_FROM(EntityType::THROWN_EGG, directEntity->GetType())
                || eTYPE_DERIVED_FROM(EntityType::PRIMED_TNT, directEntity->GetType())
                || (eTYPE_DERIVED_FROM(EntityType::ARROW, directEntity->GetType())
                    && Minecraft::GetInstance()->GetMiniGame()->HasFeatureArrowCanKnockback()))
            {
                return LivingEntity::hurt(source, damage);
            }
        }
    }

    if (isInvulnerableTo(source))
        return false;
    if (mAbilities.mInvulnerable && !source->isBypassInvul())
        return false;
    if (hasInvulnerablePrivilege())
        return false;
    if (mAbilities.mInvulnerable && !source->isBypassInvul())
        return false;
    if (isSpectator())
        return false;
    if (source == DamageSource::FALL && isAllowedToFly() && mAbilities.mFlying)
        return false;

    mNoActionTime = 0;
    if (getHealth() <= 0.0f)
        return false;

    if (isSleeping() && !mLevel->mIsClientSide)
        stopSleepInBed(true, true, false);
    removeEntitiesOnShoulder();

    if (source->scalesWithDifficulty())
    {
        if (mLevel->getDifficulty() == Difficulty::PEACEFUL)
        {
            damage = 0.0f;
        }
        if (mLevel->getDifficulty() == Difficulty::EASY)
        {
            damage = std::min(damage * 0.5f + 1.0f, damage);
        }
        if (mLevel->getDifficulty() == Difficulty::HARD)
        {
            damage = damage * 3.0f * 0.5f;
        }
    }

    if (damage == 0.0f)
        return false;

    boost::shared_ptr<Entity> attacker = source->getEntity();
    if (attacker && eTYPE_DERIVED_FROM(EntityType::ARROW, attacker->GetType()))
    {
        boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(attacker);
        if (arrow->mOwner)
        {
            attacker = arrow->mOwner;
        }
    }

    return LivingEntity::hurt(source, damage);
}

void Player::blockUsingShield(const boost::shared_ptr<LivingEntity>& attacker)
{
    LivingEntity::blockUsingShield(attacker);
    if (dynamic_cast<AxeItem*>(attacker->getMainHandItem()->getItem()) != NULL)
    {
        disableShield(true);
    }
}

void Player::actuallyHurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return;
    }

    damage = getDamageAfterArmorAbsorb(source, damage);
    damage = getDamageAfterMagicAbsorb(source, damage);
    float originalDamage = damage;
    damage = std::max(damage - getAbsorptionAmount(), 0.0f);
    setAbsorptionAmount(getAbsorptionAmount() - (originalDamage - damage));
    if (damage == 0.0f)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        {
            getCombatTracker()->recordDamage(source, getHealth(), 0.0f);
        }
        return;
    }

    causeFoodExhaustion(source->getFoodExhaustion());
    float health = getHealth();
    setHealth(getHealth() - damage);
    getCombatTracker()->recordDamage(source, health, damage);
}

void Player::checkRidingStatistiscs(double x, double y, double z)
{
    if (!isPassenger())
        return;

    int cm = static_cast<int>(Math::round(std::sqrt(x * x + y * y + z * z) * 100.0));
    if (cm <= 0)
        return;

    if (eTYPE_DERIVED_FROM(EntityType::MINECART, getVehicle()->GetType()))
    {
        mMinecartDistanceCm += cm;
        if (mMinecartDistanceCm >= 100)
        {
            int distance = mMinecartDistanceCm - mMinecartDistanceCm % 100;
            mMinecartDistanceCm -= distance;
            awardStat(GenericStats::minecartOneM(), GenericStats::param_minecart(distance / 100));
        }

        if (mStartRidingPos == NULL)
        {
            mStartRidingPos = new BlockPos(shared_from_this());
            return;
        }

        double distance = mStartRidingPos->dist(Mth::floor(mX), Mth::floor(mY), Mth::floor(mZ));
        if (!mField0x781 && static_cast<int>(distance) >= 500)
        {
            awardStat(GenericStats::onARail(), GenericStats::param_onARail(static_cast<int>(distance)));
            mField0x781 = 1;
        }
    }
    else if (eTYPE_DERIVED_FROM(EntityType::BOAT, getVehicle()->GetType()))
    {
        mBoatDistanceCm += cm;
        if (mBoatDistanceCm >= 100)
        {
            int distance = mBoatDistanceCm - mBoatDistanceCm % 100;
            mBoatDistanceCm -= distance;
            awardStat(GenericStats::boatOneM(), GenericStats::param_boat(distance / 100));
        }
    }
    else if (eTYPE_DERIVED_FROM(EntityType::PIG, getVehicle()->GetType()))
    {
        mPigDistanceCm += cm;
        if (mPigDistanceCm >= 100)
        {
            int distance = mPigDistanceCm - mPigDistanceCm % 100;
            mPigDistanceCm -= distance;
            awardStat(GenericStats::pigOneM(), GenericStats::param_pig(distance / 100));
        }
    }
}

void Player::checkMovementStatistiscs(double x, double y, double z)
{
    if (isPassenger())
    {
        if (getVehicle()->GetType() == EntityType::BOAT)
            ResetTimerAchievement(eTimerAchievement_SleepWithTheFishes);
        return;
    }

    bool underWater = isUnderLiquid(Material::WATER);
    if (!isAlive() || !isInWater() || !underWater)
    {
        ResetTimerAchievement(eTimerAchievement_SleepWithTheFishes);
    }

    if (underWater)
    {
        int distance = static_cast<int>(Math::round(std::sqrt(z * z + x * x + y * y) * 100.0));
        if (distance > 0)
        {
            mDiveDistanceCm += distance;
            if (mDiveDistanceCm >= 100)
            {
                int meters = mDiveDistanceCm / 100;
                mDiveDistanceCm %= 100;
                GameMode* gameMode = GetGameMode();
                if (gameMode != NULL)
                    gameMode->RecordSwim(meters, underWater);
            }
            causeFoodExhaustion(static_cast<float>(FoodConstants::ExhaustionSwim() * distance) * 0.01f);
            causeFoodExhaustion(FoodConstants::ExhaustionIdle());
            return;
        }
    }
    else if (isInWater())
    {
        int distance = static_cast<int>(Math::round(std::sqrt(x * x + z * z) * 100.0));
        if (distance > 0)
        {
            mSwimDistanceCm += distance;
            if (mSwimDistanceCm >= 100)
            {
                int meters = mSwimDistanceCm / 100;
                mSwimDistanceCm %= 100;
                awardStat(gStats->get_swimOneM(), GenericStats::param_swim(meters));
                GameMode* gameMode = GetGameMode();
                if (gameMode != NULL)
                    gameMode->RecordSwim(meters, false);
            }
            causeFoodExhaustion(static_cast<float>(FoodConstants::ExhaustionSwim() * distance) * 0.01f);
            causeFoodExhaustion(FoodConstants::ExhaustionIdle());
            return;
        }
    }
    else if (onLadder())
    {
        if (y > 0.0)
        {
            mClimbDistanceCm += static_cast<int>(Math::round(y * 100.0));
            if (mClimbDistanceCm >= 100)
            {
                int meters = mClimbDistanceCm / 100;
                mClimbDistanceCm %= 100;
                awardStat(gStats->get_climbOneM(), GenericStats::param_climb(meters));
                GameMode* gameMode = GetGameMode();
                if (gameMode != NULL)
                    gameMode->RecordClimb(meters);
            }
        }
    }
    else if (mOnGround)
    {
        int distance = static_cast<int>(Math::round(std::sqrt(x * x + z * z) * 100.0));
        if (distance > 0)
        {
            mWalkDistanceCm += distance;
            if (mWalkDistanceCm >= 100)
            {
                int meters = mWalkDistanceCm / 100;
                mWalkDistanceCm %= 100;
                awardStat(gStats->get_walkOneM(), GenericStats::param_walk(meters));
                GameMode* gameMode = GetGameMode();
                if (gameMode != NULL)
                    gameMode->RecordWalk(meters, isSprinting(), isSneaking());
            }
            if (isSprinting())
            {
                causeFoodExhaustion(static_cast<float>(FoodConstants::ExhaustionSprint() * distance) * 0.01f);
            }
            else if (isSneaking())
            {
                causeFoodExhaustion(static_cast<float>(FoodConstants::ExhaustionSneak() * distance) * 0.01f);
            }
            else
            {
                causeFoodExhaustion(static_cast<float>(FoodConstants::ExhaustionWalk() * distance) * 0.01f);
            }
        }
    }

    causeFoodExhaustion(FoodConstants::ExhaustionIdle());
}

void Player::causeFallDamage(float distance, float multiplier)
{
    if (mAbilities.mMayfly)
    {
        MobEffectInstance* jumpBoost = getEffect(MobEffects::JUMP);
        float jumpReduction = jumpBoost != NULL ? jumpBoost->getAmplifier() + 1.0f : 0.0f;
        int damage = static_cast<int>(std::ceil((distance - 3.0f - jumpReduction) * multiplier));
        if (damage > 0)
            playSound(getFallDamageSound(damage), 1.0f, 1.0f);
        return;
    }

    if (distance >= 2.0f)
    {
        mFallDistanceCm += static_cast<int>(Math::round(distance * 100.0));
        if (mFallDistanceCm >= 100)
        {
            int newDistance = mFallDistanceCm - mFallDistanceCm % 100;
            mFallDistanceCm -= newDistance;
            awardStat(GenericStats::fallOneM(), GenericStats::param_fall(newDistance / 100));
        }
    }
    LivingEntity::causeFallDamage(distance, multiplier);
}

void Player::travel(float x, float y, float z)
{
    double oldX = mX;
    double oldY = mY;
    double oldZ = mZ;

    if (isSwimming())
    {
        Vec3* look = getLookAngle();
        float swimFollow = look->y < -0.2f ? 0.085f : 0.06f;
        if (mAbilities.mFlying)
            swimFollow = 1.3f;
        Vec3* pos = getPos(0.0f);
        Material* feetMaterial = mLevel->getBlockState(BlockPos(pos))->getMaterial();
        pos->y += 1.62f - 1.2f;
        Material* headMaterial = mLevel->getBlockState(BlockPos(pos))->getMaterial();
        bool follow = look->y <= 0.0 || mAbilities.mFlying || mJumping || headMaterial->isLiquid();
        if (follow && feetMaterial == Material::AIR && look->y > 0.0 && !isSneaking())
            follow = false;
        if (follow)
            mYd += (look->y - mYd) * swimFollow;
        else
            mYd = 0.0;
    }

    if (mAbilities.mFlying && !isPassenger())
    {
        double oldYd = mYd;
        float oldFlyingSpeed = mFlyingSpeed;
        mFlyingSpeed = mAbilities.getFlyingSpeed() * (isSprinting() ? 2.0f : 1.0f);
        LivingEntity::travel(x, y, z);
        mYd = oldYd * 0.6;
        mFlyingSpeed = oldFlyingSpeed;
        mFallDistance = 0.0f;
        setSharedFlag(FLAG_FALL_FLYING, false);
    }
    else if (isFallFlying() && mLevel->mIsClientSide)
    {
        double fromX = mX;
        double fromY = mY;
        double fromZ = mZ;
        LivingEntity::travel(x, y, z);
        fjCheckSuperSonic(fromX, fromY, fromZ, mX, mY, mZ);
    }
    else
    {
        LivingEntity::travel(x, y, z);
    }
    checkMovementStatistiscs(mX - oldX, mY - oldY, mZ - oldZ);
}

void Player::playShoulderEntityAmbientSound(CompoundTag* tag)
{
    if (tag == NULL || (tag->contains(L"Silent") && tag->getBoolean(L"Silent")))
    {
        return;
    }
    if (tag->getString(L"id") == EntityIO::getKey(EntityType::PARROT).toString())
    {
        Parrot::playAmbientSound(mLevel, shared_from_this());
    }
}

void Player::respawnEntityOnShoulder(CompoundTag* tag)
{
    if (mLevel->mIsClientSide || tag->isEmpty())
        return;
    boost::shared_ptr<Entity> entity = EntityIO::newEntity(tag, mLevel);
    if (eTYPE_DERIVED_FROM(EntityType::TAMABLE_ANIMAL, entity->GetType()))
        boost::static_pointer_cast<TamableAnimal>(entity)->setOwnerUUID(getUUID());
    entity->setPos(mX, mY + 0.699999988079071, mZ);
    mLevel->addEntity(entity);
}

void Player::removeEntitiesOnShoulder()
{
    respawnEntityOnShoulder(getShoulderEntityLeft());
    if (!getShoulderEntityLeft()->isEmpty())
        setShoulderEntityLeft(new CompoundTag());
    respawnEntityOnShoulder(getShoulderEntityRight());
    if (!getShoulderEntityRight()->isEmpty())
        setShoulderEntityRight(new CompoundTag());
}

void Player::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    LivingEntity::onSyncedDataUpdated(accessor);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (accessor->getId() == DATA_SHARED_FLAGS_ID->getId() && isFallFlying() && !mWasFallFlying
        && mLevel->mIsClientSide)
    {
        ElytraOnPlayerSoundInstance* sound = new ElytraOnPlayerSoundInstance(
            boost::static_pointer_cast<Player>(shared_from_this()));
        minecraft->mSoundEngine->ConsoleSoundEngine::play(sound);
    }
}

void Player::remove()
{
    Entity::remove();
    mInventoryMenu->removed(boost::static_pointer_cast<Player>(shared_from_this()));
    if (mContainerMenu != NULL)
    {
        mContainerMenu->removed(boost::static_pointer_cast<Player>(shared_from_this()));
        mContainerMenu = mInventoryMenu;
    }
}

uint32_t Player::startSleepInBed(const BlockPos& position, bool force)
{
    const Direction* direction = Direction::NORTH;
    const BlockState* bedState = mLevel->getBlockState(position);
    if (bedState->getBlock() == Blocks::bed)
        direction = bedState->getValue<const Direction*>(HorizontalDirectionalBlock::FACING);

    if (!mLevel->mIsClientSide || force)
    {
        const BlockState* state = mLevel->getBlockState(position);
        if ((mLevel->isUnderWater(position, state) && !hasEffect(MobEffects::WATER_BREATHING)
             && !hasEffect(MobEffects::CONDUIT_POWER))
            || isSleeping() || !isAlive())
        {
            return BED_SLEEPING_OTHER_PROBLEM;
        }
        if (!mLevel->mDimension->isNaturalDimension())
            return BED_SLEEPING_NOT_POSSIBLE_HERE;
        if (!bedInRange(position, direction))
            return BED_SLEEPING_TOO_FAR_AWAY;
        if (force)
            return BED_SLEEPING_OK;

        PreventsMeFromSleeping preventsSleeping(boost::static_pointer_cast<Player>(shared_from_this()));
        double x = position.x;
        double y = position.y;
        double z = position.z;
        std::vector<boost::shared_ptr<Entity> >* monsters = mLevel->getEntitiesOfClass(
            typeid(Monster), AABB::newTemp(x - 8.0, y - 5.0, z - 8.0, x + 8.0, y + 5.0, z + 8.0), &preventsSleeping,
            false);
        bool monstersNearby = !monsters->empty() && !mAbilities.mInstabuild;
        delete monsters;
        if (monstersNearby)
            return BED_SLEEPING_NOT_SAFE;
        if (mLevel->isDay())
            return BED_SLEEPING_NOT_POSSIBLE_NOW;
    }

    if (isPassenger())
        stopRiding();
    removeEntitiesOnShoulder();
    SetNormalSize();
    setSize(0.2f, 0.2f);
    if (mLevel->hasChunkAt(position))
    {
        float offsetX = 0.5f;
        float offsetZ = 0.5f;
        if (mLevel->getBlockState(position)->getBlock() == Blocks::bed)
        {
            offsetX = direction->getStepX() * 0.4f + 0.5f;
            offsetZ = direction->getStepZ() * 0.4f + 0.5f;
            setBedOffset(direction);
        }
        setPos(static_cast<float>(position.x) + offsetX, static_cast<float>(position.y) + 0.6875f,
               static_cast<float>(position.z) + offsetZ);
    }
    else
    {
        setPos(static_cast<float>(position.x) + 0.5f, static_cast<float>(position.y) + 0.6875f,
               static_cast<float>(position.z) + 0.5f);
    }
    mSleeping = true;
    mSleepCounter = 0;
    if (mBedPosition == NULL)
        mBedPosition = new BlockPos();
    *mBedPosition = position;
    mXd = 0.0;
    mZd = 0.0;
    mYd = 0.0;
    if (!mLevel->mIsClientSide)
        mLevel->updateSleepingPlayerList();
    mOAttackAnim = 0.0f;
    mAttackAnim = 0.0f;
    return BED_SLEEPING_OK;
}

void Player::stopSleepInBed(bool resetTimer, bool updateLevel, bool setSpawn)
{
    SetNormalSize();
    setSize(0.6f, 1.8f);
    if (mBedPosition != NULL)
    {
        const BlockState* state = mLevel->getBlockState(*mBedPosition);
        if (state->getBlock() == Blocks::bed)
        {
            TypedBoxed<bool> vacant(false);
            mLevel->setBlock(*mBedPosition, state->setBoxedValue(BedBlock::OCCUPIED, &vacant), Block::UPDATE_INVISIBLE,
                             false);
            BlockPos* standUpPos = BedBlock::findStandUpPosition(mLevel, *mBedPosition, 0);
            if (standUpPos == NULL)
                standUpPos = new BlockPos(mBedPosition->above());
            setPos(static_cast<float>(standUpPos->x) + 0.5f, static_cast<float>(standUpPos->y) + 0.1f,
                   static_cast<float>(standUpPos->z) + 0.5f);
        }
    }
    mSleeping = false;
    if (!mLevel->mIsClientSide && updateLevel)
        mLevel->updateSleepingPlayerList();
    mSleepCounter = resetTimer ? 0 : 100;
    if (setSpawn)
        setRespawnPosition(new BlockPos(*mBedPosition), false);
}

BlockPos* Player::checkBedValidRespawnPosition(Level* level, const BlockPos& position, bool forced)
{
    ChunkSource* chunkSource = level->getChunkSource();
    chunkSource->create((position.x - 3) >> 4, (position.z - 3) >> 4);
    chunkSource->create((position.x + 3) >> 4, (position.z - 3) >> 4);
    chunkSource->create((position.x - 3) >> 4, (position.z + 3) >> 4);
    chunkSource->create((position.x + 3) >> 4, (position.z + 3) >> 4);

    if (level->getBlockState(position)->getBlock() == Blocks::bed)
    {
        return BedBlock::findStandUpPosition(level, position, 0);
    }

    if (!forced)
    {
        return NULL;
    }

    bool freeAtFeet = level->getBlockState(position)->getBlock()->isPossibleToRespawnInThis();
    bool freeAtHead = level->getBlockState(position.above())->getBlock()->isPossibleToRespawnInThis();
    if (freeAtFeet && freeAtHead)
    {
        return new BlockPos(position);
    }
    return NULL;
}

void Player::fjCheckSuperSonic(double oldX, double oldY, double oldZ, double newX, double newY, double newZ)
{
    static double minSpeedSqr = std::pow(2.0, 2);

    double speedSqr = mYd * mYd + mXd * mXd + mZd * mZd;
    double blocksPerSecond = std::sqrt(speedSqr) * 20.0;
    if (speedSqr <= minSpeedSqr)
    {
        return;
    }

    double largestComponent = std::max(std::fabs(mXd), std::max(std::fabs(mYd), std::fabs(mZd)));
    double step = 0.2 / largestComponent;

    double xDistance = std::fabs(newX - oldX);
    double yDistance = std::fabs(newY - oldY);
    double zDistance = std::fabs(newZ - oldZ);
    Direction::Axis* axis;
    if (xDistance >= yDistance && xDistance >= zDistance)
    {
        axis = Direction::Axis::X;
    }
    else if (zDistance <= yDistance)
    {
        axis = Direction::Axis::Y;
    }
    else
    {
        axis = Direction::Axis::Z;
    }

    const Direction* firstSide = Direction::get(Direction::AxisDirection::POSITIVE,
                                                Direction::Axis::VALUES[(axis->getId() + 1) % 3]);
    const Direction* oppositeFirstSide = Direction::get(Direction::AxisDirection::NEGATIVE,
                                                        Direction::Axis::VALUES[(axis->getId() + 1) % 3]);
    const Direction* secondSide = Direction::get(Direction::AxisDirection::POSITIVE,
                                                 Direction::Axis::VALUES[(axis->getId() + 2) % 3]);
    const Direction* oppositeSecondSide = Direction::get(Direction::AxisDirection::NEGATIVE,
                                                         Direction::Axis::VALUES[(axis->getId() + 2) % 3]);

    BlockPos previous(0, -1, 0);
    int speed = (int)blocksPerSecond;
    for (double t = 0.0; t != 1.0; t = std::min(t + step, 1.0))
    {
        BlockPos pos((newX - oldX) * t + oldX, (newY - oldY) * t + oldY, (newZ - oldZ) * t + oldZ);
        if (pos != previous)
        {
            if (mLevel->isSolidBlockingBlockInLoadedChunk(pos.relative(firstSide), false)
                && mLevel->isSolidBlockingBlockInLoadedChunk(pos.relative(oppositeFirstSide), false)
                && mLevel->isSolidBlockingBlockInLoadedChunk(pos.relative(secondSide), false)
                && mLevel->isSolidBlockingBlockInLoadedChunk(pos.relative(oppositeSecondSide), false))
            {
                awardStat(GenericStats::superSonic(), GenericStats::param_superSonic(speed));
            }
            previous = pos;
        }
    }
}

void Player::killed(boost::shared_ptr<LivingEntity> entity)
{
    bool isEnemy = (EntityType::ENEMY & entity->GetType()) == EntityType::ENEMY;
    if (!isEnemy && entity->GetType() != EntityType::GHAST && entity->GetType() != EntityType::SLIME
        && entity->GetType() != EntityType::MAGMA_CUBE && entity->GetType() != EntityType::ENDER_DRAGON)
    {
        return;
    }

    awardStat(GenericStats::killEnemy(), GenericStats::param_noArgs());
    not_null_ptr<ItemInstance> helmet = getItemSlot(EquipmentSlot::HEAD);
    switch (entity->GetType())
    {
    case EntityType::CREEPER:
    {
        Stat* stat = gStats->get_killsCreeper();
        awardStat(stat, GenericStats::param_noArgs());
        if (helmet && helmet->getItem() == Items::skull)
        {
            if (helmet->getItem()->getDescriptionId(getItemSlot(EquipmentSlot::HEAD)) == IDS_ITEM_SKULL_CREEPER)
            {
                awardStat(GenericStats::camouflage(), GenericStats::param_camouflage());
            }
        }
        break;
    }
    case EntityType::SKELETON:
    case EntityType::WITHER_SKELETON:
    case EntityType::STRAY:
    {
        bool jockey = false;
        if (entity->isPassenger() && (EntityType::SPIDER & entity->getVehicle()->GetType()) == EntityType::SPIDER)
        {
            jockey = true;
        }
        if (jockey)
        {
            Stat* stat = gStats->get_killsSpiderJockey();
            awardStat(stat, GenericStats::param_noArgs());
        }
        else
        {
            Stat* stat = gStats->get_killsSkeleton();
            awardStat(stat, GenericStats::param_noArgs());
        }
        if (helmet && helmet->getItem() == Items::skull)
        {
            int skullName = IDS_ITEM_SKULL_SKELETON;
            if (EntityType::WITHER_SKELETON == entity->GetType())
                skullName = IDS_ITEM_SKULL_WITHER;
            if (helmet->getItem()->getDescriptionId(getItemSlot(EquipmentSlot::HEAD)) == skullName)
            {
                awardStat(GenericStats::camouflage(), GenericStats::param_camouflage());
            }
        }
        break;
    }
    case EntityType::SPIDER:
    {
        bool jockey = entity->getControllingPassenger()
                      && entity->getControllingPassenger()->GetType() == EntityType::SKELETON;
        if (jockey)
        {
            Stat* stat = gStats->get_killsSpiderJockey();
            awardStat(stat, GenericStats::param_noArgs());
        }
        else
        {
            Stat* stat = gStats->get_killsSpider();
            awardStat(stat, GenericStats::param_noArgs());
        }
        break;
    }
    case EntityType::ZOMBIE:
    {
        Stat* stat = gStats->get_killsZombie();
        awardStat(stat, GenericStats::param_noArgs());
        if (helmet && helmet->getItem() == Items::skull)
        {
            if (helmet->getItem()->getDescriptionId(getItemSlot(EquipmentSlot::HEAD)) == IDS_ITEM_SKULL_ZOMBIE)
            {
                awardStat(GenericStats::camouflage(), GenericStats::param_camouflage());
            }
        }
        break;
    }
    case EntityType::PIG_ZOMBIE:
        if (mLevel->mDimension->getType() == DimensionType::OVERWORLD)
        {
            Stat* stat = gStats->get_killsZombiePigman();
            awardStat(stat, GenericStats::param_noArgs());
        }
        else
        {
            Stat* stat = gStats->get_killsNetherZombiePigman();
            awardStat(stat, GenericStats::param_noArgs());
        }
        break;
    case EntityType::GHAST:
    {
        Stat* stat = gStats->get_killsGhast();
        awardStat(stat, GenericStats::param_noArgs());
        break;
    }
    case EntityType::SLIME:
    {
        Stat* stat = gStats->get_killsSlime();
        awardStat(stat, GenericStats::param_noArgs());
        break;
    }
    case EntityType::ENDER_DRAGON:
    {
        Stat* stat = gStats->get_killsEnderdragon();
        awardStat(stat, GenericStats::param_noArgs());
        break;
    }
    case EntityType::ELDER_GUARDIAN:
        awardStat(GenericStats::theDeepEnd(), GenericStats::param_noArgs());
        break;
    case EntityType::EVOCATION_ILLAGER:
        awardStat(GenericStats::feelingIll(), GenericStats::param_noArgs());
        break;
    }
}

TextureAtlasSprite* Player::getItemInHandIcon(not_null_ptr<ItemInstance> item, int frame)
{
    TextureAtlasSprite* icon = LivingEntity::getItemInHandIcon(item, frame);
    if (item->getItem() == Items::fishingRod && mFishing != NULL)
    {
        icon = static_cast<FishingRodItem*>(Items::fishingRod)->getEmptyIcon();
    }
    else if (item->getItem()->hasMultipleSpriteLayers())
    {
        return item->getItem()->getLayerIcon(item->getAuxValue(), frame, item);
    }
    else if (item->getItem() == Items::bow)
    {
        int ticksHeld = getTicksUsingItem();
        if (ticksHeld >= BowItem::GetMaxBowDuration() - 2)
            return static_cast<BowItem*>(Items::bow)->getDrawnIcon(2);
        if (ticksHeld > BowItem::GetMaxBowDuration() * 2 / 3)
            return static_cast<BowItem*>(Items::bow)->getDrawnIcon(1);
        if (ticksHeld > 0)
            return static_cast<BowItem*>(Items::bow)->getDrawnIcon(0);
    }
    return icon;
}

bool Player::mayUseItemAt(const BlockPos& position, const Direction* direction, not_null_ptr<ItemInstance> item)
{
    if (mAbilities.mMayBuild)
        return 1;
    if (!item->isEmpty())
    {
        GameMode* gameMode = GetGameMode();
        if (gameMode != NULL)
            return gameMode->MayUse(position, item->getItem());
        return item->mayBePlacedInAdventureMode();
    }
    BlockPos adjacent = position.relative(direction->getOpposite());
    Block* block = mLevel->getBlockState(adjacent)->getBlock();
    return item->hasAdventureModePlaceTagForBlock(block) || item->mayBePlacedInAdventureMode();
}

uint32_t Player::isAllowedToUse(Block* block)
{
    unsigned int id = Block::getId(block);
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        return Minecraft::GetInstance()->GetMiniGame()->MayUseBlock(static_cast<int>(id));
    if (block == NULL)
        return 1;
    if (gApp.GetGameHostOption(eGameHostOption_TrustPlayers) != 0)
        return 1;

    bool usableBySwitchOrDoor = false;
    if (getPlayerGamePrivilege(ePlayerGamePrivilege_CanUseDoorsAndSwitches) != 0)
    {
        bool listedBlock = true;
        if (id < BlockId::UNPOWERED_COMPARATOR)
        {
            if (id != BlockId::LEVER && id != BlockId::STONE_BUTTON && id != BlockId::TRAPDOOR
                && id != BlockId::WOODEN_BUTTON)
                listedBlock = false;
        }
        else if (id > BlockId::DAYLIGHT_DETECTOR && id != BlockId::IRON_TRAPDOOR
                 && id != BlockId::DAYLIGHT_DETECTOR_INVERTED
                 && (id < BlockId::ACACIA_BUTTON || id > BlockId::SPRUCE_BUTTON))
        {
            listedBlock = false;
        }
        if (listedBlock)
        {
            usableBySwitchOrDoor = true;
        }
        else
        {
            usableBySwitchOrDoor = DoorBlock::isDoor(block) || FenceGateBlock::isFenceGate(block);
        }
    }

    if (getPlayerGamePrivilege(ePlayerGamePrivilege_CanUseContainers) != 0)
    {
        switch (id)
        {
        case BlockId::DISPENSER:
        case BlockId::CHEST:
        case BlockId::CRAFTING_TABLE:
        case BlockId::FURNACE:
        case BlockId::LIT_FURNACE:
        case BlockId::ENCHANTING_TABLE:
        case BlockId::BREWING_STAND:
        case BlockId::ENDER_CHEST:
        case BlockId::BEACON:
        case BlockId::ANVIL:
        case BlockId::TRAPPED_CHEST:
        case BlockId::HOPPER:
        case BlockId::DROPPER:
        case BlockId::WHITE_SHULKER_BOX:
        case BlockId::ORANGE_SHULKER_BOX:
        case BlockId::MAGENTA_SHULKER_BOX:
        case BlockId::LIGHT_BLUE_SHULKER_BOX:
        case BlockId::YELLOW_SHULKER_BOX:
        case BlockId::LIME_SHULKER_BOX:
        case BlockId::PINK_SHULKER_BOX:
        case BlockId::GRAY_SHULKER_BOX:
        case BlockId::SILVER_SHULKER_BOX:
        case BlockId::CYAN_SHULKER_BOX:
        case BlockId::PURPLE_SHULKER_BOX:
        case BlockId::BLUE_SHULKER_BOX:
        case BlockId::BROWN_SHULKER_BOX:
        case BlockId::GREEN_SHULKER_BOX:
        case BlockId::RED_SHULKER_BOX:
        case BlockId::BLACK_SHULKER_BOX:
            return 1;
        }
    }
    if (usableBySwitchOrDoor)
        return 1;
    if (getPlayerGamePrivilege(ePlayerGamePrivilege_CannotBuild) != 0)
        return 0;

    switch (id)
    {
    case BlockId::DISPENSER:
    case BlockId::CHEST:
    case BlockId::CRAFTING_TABLE:
    case BlockId::FURNACE:
    case BlockId::LIT_FURNACE:
    case BlockId::ENCHANTING_TABLE:
    case BlockId::BREWING_STAND:
    case BlockId::ENDER_CHEST:
    case BlockId::BEACON:
    case BlockId::ANVIL:
    case BlockId::TRAPPED_CHEST:
    case BlockId::HOPPER:
    case BlockId::DROPPER:
    case BlockId::WHITE_SHULKER_BOX:
    case BlockId::ORANGE_SHULKER_BOX:
    case BlockId::MAGENTA_SHULKER_BOX:
    case BlockId::LIGHT_BLUE_SHULKER_BOX:
    case BlockId::YELLOW_SHULKER_BOX:
    case BlockId::LIME_SHULKER_BOX:
    case BlockId::PINK_SHULKER_BOX:
    case BlockId::GRAY_SHULKER_BOX:
    case BlockId::SILVER_SHULKER_BOX:
    case BlockId::CYAN_SHULKER_BOX:
    case BlockId::PURPLE_SHULKER_BOX:
    case BlockId::BLUE_SHULKER_BOX:
    case BlockId::BROWN_SHULKER_BOX:
    case BlockId::GREEN_SHULKER_BOX:
    case BlockId::RED_SHULKER_BOX:
    case BlockId::BLACK_SHULKER_BOX:
    case BlockId::LEVER:
    case BlockId::STONE_BUTTON:
    case BlockId::TRAPDOOR:
    case BlockId::WOODEN_BUTTON:
    case BlockId::UNPOWERED_COMPARATOR:
    case BlockId::POWERED_COMPARATOR:
    case BlockId::DAYLIGHT_DETECTOR:
    case BlockId::IRON_TRAPDOOR:
    case BlockId::DAYLIGHT_DETECTOR_INVERTED:
    case BlockId::ACACIA_BUTTON:
    case BlockId::BIRCH_BUTTON:
    case BlockId::DARK_OAK_BUTTON:
    case BlockId::JUNGLE_BUTTON:
    case BlockId::SPRUCE_BUTTON:
        return 0;
    }
    return (DoorBlock::isDoor(block) || FenceGateBlock::isFenceGate(block)) ? 0 : 1;
}

uint32_t Player::isAllowedToUse(not_null_ptr<ItemInstance> item)
{
    unsigned int id = Item::getId(item->getItem());
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        return Minecraft::GetInstance()->GetMiniGame()->MayUse(static_cast<int>(id));
    bool allowed = true;
    if (!item->isEmpty() && gApp.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
    {
        if (getPlayerGamePrivilege(ePlayerGamePrivilege_CannotBuild) != 0)
            allowed = false;
        switch (id)
        {
        case ItemId::APPLE:
        case ItemId::BOW:
        case ItemId::IRON_SWORD:
        case ItemId::WOODEN_SWORD:
        case ItemId::STONE_SWORD:
        case ItemId::DIAMOND_SWORD:
        case ItemId::MUSHROOM_STEW:
        case ItemId::GOLDEN_SWORD:
        case ItemId::BREAD:
        case ItemId::LEATHER_HELMET:
        case ItemId::LEATHER_CHESTPLATE:
        case ItemId::LEATHER_LEGGINGS:
        case ItemId::LEATHER_BOOTS:
        case ItemId::CHAINMAIL_HELMET:
        case ItemId::CHAINMAIL_CHESTPLATE:
        case ItemId::CHAINMAIL_LEGGINGS:
        case ItemId::CHAINMAIL_BOOTS:
        case ItemId::IRON_HELMET:
        case ItemId::IRON_CHESTPLATE:
        case ItemId::IRON_LEGGINGS:
        case ItemId::IRON_BOOTS:
        case ItemId::DIAMOND_HELMET:
        case ItemId::DIAMOND_CHESTPLATE:
        case ItemId::DIAMOND_LEGGINGS:
        case ItemId::DIAMOND_BOOTS:
        case ItemId::GOLDEN_HELMET:
        case ItemId::GOLDEN_CHESTPLATE:
        case ItemId::GOLDEN_LEGGINGS:
        case ItemId::GOLDEN_BOOTS:
        case ItemId::PORKCHOP:
        case ItemId::COOKED_PORKCHOP:
        case ItemId::GOLDEN_APPLE:
        case ItemId::FISH:
        case ItemId::COOKED_FISH:
        case ItemId::COOKIE:
        case ItemId::MELON:
        case ItemId::BEEF:
        case ItemId::COOKED_BEEF:
        case ItemId::CHICKEN:
        case ItemId::COOKED_CHICKEN:
        case ItemId::ROTTEN_FLESH:
        case ItemId::POTION:
        case ItemId::EXPERIENCE_BOTTLE:
        case ItemId::SPLASH_POTION:
        case ItemId::LINGERING_POTION:
            allowed = true;
            break;
        }
    }
    return allowed;
}

bool Player::isAllowedToInteract(boost::shared_ptr<Entity> entity)
{
    bool allowed = true;

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity->GetType()))
    {
        return false;
    }

    if (gApp.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
    {
        if (eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
        {
            if (getPlayerGamePrivilege(ePlayerGamePrivilege_CanUseContainers) == 0)
            {
                boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
                switch (minecart->getType()->mId)
                {
                case Minecart::Type::eType_Chest:
                case Minecart::Type::eType_Hopper:
                    allowed = false;
                    break;
                }
            }
        }
        else if (eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, entity->GetType()))
        {
            if (getPlayerGamePrivilege(ePlayerGamePrivilege_CanUseContainers) == 0)
            {
                boost::shared_ptr<AbstractHorse> horse = boost::static_pointer_cast<AbstractHorse>(entity);
                bool ownedByOther = false;
                if (horse->isTamed() && horse->getOwnerUUID() != getUUID())
                {
                    ownedByOther = true;
                }
                if (ownedByOther)
                {
                    allowed = false;
                }
            }
        }
        else if (eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity->GetType()))
        {
            if (getPlayerGamePrivilege(ePlayerGamePrivilege_CanUseContainers) == 0)
            {
                allowed = false;
            }
        }
        else
        {
            if (getPlayerGamePrivilege(ePlayerGamePrivilege_CannotBuild) != 0)
            {
                allowed = false;
            }
            if (getPlayerGamePrivilege(ePlayerGamePrivilege_CannotMine) != 0)
            {
                allowed = false;
            }
        }
    }

    return allowed;
}

uint32_t Player::isAllowedToHurtEntity(boost::shared_ptr<Entity> entity)
{
    const unsigned long long type = entity->GetType();
    bool allowed = true;

    if (!isAllowedToMine() || gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        switch (type)
        {
        case EntityType::ARMOR_STAND:
        case EntityType::LEASH_FENCE_KNOT_ENTITY:
        case EntityType::ITEM_FRAME:
        case EntityType::PAINTING:
        case EntityType::HANGING_ENTITY:
        case EntityType::BOAT:
            allowed = false;
            break;
        default:
            allowed = !eTYPE_DERIVED_FROM(EntityType::MINECART, type);
            break;
        }
    }

    if (allowed)
    {
        if (eTYPE_DERIVED_FROM(EntityType::ANIMAL, type))
        {
            allowed = isAllowedToAttackAnimals();
        }
        else if (eTYPE_DERIVED_FROM(EntityType::PLAYER, type))
        {
            boost::shared_ptr<Player> target = boost::dynamic_pointer_cast<Player>(entity);
            allowed = isAllowedToAttackPlayers() && gApp.GetGameHostOption(eGameHostOption_PvP) != 0
                      && (GetGameMode() == NULL || GetGameType() != GameType::LOBBY)
                      && !Minecraft::InMiniGame(eMiniGameId_Glide, true) && !target->mAbilities.mInstabuild
                      && !target->isSpectator();
        }
    }

    return allowed;
}

uint8_t Player::SpectateAsSkull()
{
    uint32_t spectating = 0;
    if (SpectateAsFloatingHead())
    {
        if (std::find(sDlcSkinIds.begin(), sDlcSkinIds.end(), mCustomSkin) != sDlcSkinIds.end())
            spectating = 1;
    }
    return spectating;
}

int32_t Player::AutoEquip(not_null_ptr<ItemInstance>& item, not_null_ptr<ItemInstance>& replaced,
                          bool& replacedExisting)
{
    int32_t equipped = 0;
    replacedExisting = false;
    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || !item->getItem()->IsArmor())
        return 0;
    if (!GetGameMode()->AutoEquipEnabled())
        return 0;

    const EquipmentSlot* slot = Mob::getEquipmentSlotForItem(item);
    not_null_ptr<ItemInstance> current = getItemSlot(slot);
    if (!current->isEmpty() && current->getItem()->IsArmor())
    {
        ArmorItem* newArmor = static_cast<ArmorItem*>(item->getItem());
        ArmorItem* currentArmor = static_cast<ArmorItem*>(current->getItem());
        if (newArmor->mDefense > currentArmor->mDefense && !current->isEnchanted())
        {
            replacedExisting = true;
            equipped = setSlot(slot->getIndex() + 100, item->copy());
        }
    }
    else
    {
        if (current->getItem() == Items::skull)
            replacedExisting = true;
        equipped = setSlot(slot->getIndex() + 100, item->copy());
    }
    if (equipped)
    {
        OnEquipArmor(item);
        item->setCount(0);
    }

    if (replacedExisting)
    {
        item = current;
        replaced = current;
        equipped = 0;
    }
    return equipped;
}

uint32_t Player::interactOn(boost::shared_ptr<Entity> entity, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = getItemInHand(hand);
    boost::shared_ptr<Player> self = boost::static_pointer_cast<Player>(shared_from_this());

    if (isSpectator())
    {
        boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(entity);
        if (!container)
            return 1;
        openContainer(container);
        return 0;
    }

    not_null_ptr<ItemInstance> beforeItem = item->isEmpty() ? ItemInstance::EMPTY : item->copy();

    if (entity->interact(self, hand))
    {
        if (mAbilities.mInstabuild && item == getItemInHand(hand) && item->getCount() < beforeItem->getCount())
        {
            item->setCount(beforeItem->getCount());
        }
        return 0;
    }

    if (!item->isEmpty() && (entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        if (mAbilities.mInstabuild)
        {
            item = beforeItem;
        }
        if (item->interactEnemy(self, boost::static_pointer_cast<LivingEntity>(entity), hand))
        {
            if (item->isEmpty() && !mAbilities.mInstabuild)
            {
                setItemInHand(hand, ItemInstance::EMPTY);
            }
            return 0;
        }
    }
    return 1;
}

void Player::attack(boost::shared_ptr<Entity> entity)
{
    if (!entity->isAttackable())
    {
        return;
    }

    if (entity->skipAttackInteraction(shared_from_this()) || isSpectator())
    {
        return;
    }

    float damage = (float)getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue();
    float enchantDamage;
    int knockback = 0;

    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        boost::shared_ptr<LivingEntity> target = boost::static_pointer_cast<LivingEntity>(entity);
        enchantDamage = EnchantmentHelper::getDamageBonus(getMainHandItem(), target);
        knockback = EnchantmentHelper::getKnockbackBonus(boost::static_pointer_cast<LivingEntity>(shared_from_this()),
                                                         target);
    }
    else
    {
        enchantDamage = EnchantmentHelper::getDamageBonus(getMainHandItem(), boost::shared_ptr<LivingEntity>());
    }

    if (isSprinting())
    {
        ++knockback;
    }

    if (damage <= 0.0f && enchantDamage <= 0.0f)
    {
        return;
    }

    bool critical = mFallDistance > 0.0f && !mOnGround && !onLadder() && !isInWater()
                    && !hasEffect(MobEffects::BLINDNESS) && !isPassenger()
                    && (entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY;
    if (critical && damage > 0.0f)
    {
        damage *= 1.5f;
    }

    float totalDamage = damage + enchantDamage;
    bool startedFire = false;
    int fireAspect = EnchantmentHelper::getFireAspect(boost::static_pointer_cast<LivingEntity>(shared_from_this()));
    float healthBefore = 0.0f;

    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        healthBefore = boost::static_pointer_cast<LivingEntity>(entity)->getHealth();
        if (fireAspect > 0 && !entity->isOnFire())
        {
            startedFire = true;
            entity->setSecondsOnFire(1);
        }
    }

    double targetXd = entity->mXd;
    double targetYd = entity->mYd;
    double targetZd = entity->mZd;

    DamageSource* source = DamageSource::CreatePlayerAttack(boost::static_pointer_cast<Player>(shared_from_this()));
    bool damaged = entity->hurt(source, totalDamage);
    delete source;

    if (!damaged)
    {
        if (!isFallFlying())
        {
            mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::PLAYER_ATTACK_NODAMAGE,
                              getSoundSource(), 0.6f, 1.0f, 16.0f);
        }

        if (startedFire)
        {
            entity->clearFire();
        }
        return;
    }

    if (knockback > 0)
    {
        entity->push(-Mth::sin(mYRot * Mth::DEG_TO_RAD) * (float)knockback * 0.5f, 0.1,
                     Mth::cos(mYRot * Mth::DEG_TO_RAD) * (float)knockback * 0.5f);
        mXd *= 0.6;
        mZd *= 0.6;
        setSprinting(false);
    }

    if (entity->GetType() == EntityType::SERVER_PLAYER && entity->mHurtMarked)
    {
        boost::static_pointer_cast<ServerPlayer>(entity)->mConnection->send(
            boost::shared_ptr<Packet>(new ClientboundSetEntityMotionPacket(entity)));
        entity->mHurtMarked = false;
        entity->mXd = targetXd;
        entity->mYd = targetYd;
        entity->mZd = targetZd;
    }

    if (critical)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::PLAYER_ATTACK_CRIT, getSoundSource(),
                          1.0f, 1.0f, 16.0f);
        crit(entity);
    }
    else
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::PLAYER_ATTACK_WEAK, getSoundSource(),
                          1.0f, 1.0f, 16.0f);
    }

    if (enchantDamage > 0.0f)
    {
        magicCrit(entity);
    }

    if (totalDamage >= 18.0f)
    {
        awardStat(GenericStats::overkill(), GenericStats::param_overkill((int)(damage + enchantDamage)));
    }

    setLastHurtMob(entity);

    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        EnchantmentHelper::doPostHurtEffects(boost::static_pointer_cast<LivingEntity>(entity), shared_from_this());
    }

    EnchantmentHelper::doPostDamageEffects(boost::static_pointer_cast<LivingEntity>(shared_from_this()), entity);

    not_null_ptr<ItemInstance> item = getMainHandItem();
    boost::shared_ptr<Entity> hurtTarget = entity;

    if (entity->GetType() == EntityType::MULTI_ENTITY_MOB_PART)
    {
        boost::shared_ptr<Entity> parent = boost::dynamic_pointer_cast<Entity>(
            boost::static_pointer_cast<MultiEntityMobPart>(entity)->mParentMob.lock());
        if (parent != NULL && (parent->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
        {
            hurtTarget = boost::static_pointer_cast<LivingEntity>(parent);
        }
    }

    if (!item->isEmpty() && (hurtTarget->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        item->hurtEnemy(boost::static_pointer_cast<LivingEntity>(hurtTarget),
                        boost::static_pointer_cast<Player>(shared_from_this()));
        if (item->isEmpty())
        {
            setItemInHand(InteractionHand::eInteractionHand_MainHand, ItemInstance::EMPTY);
        }
    }

    if ((entity->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY)
    {
        float healthAfter = boost::static_pointer_cast<LivingEntity>(entity)->getHealth();

        if (fireAspect > 0)
        {
            entity->setSecondsOnFire(fireAspect * 4);
        }

        ServerLevel* serverLevel = dynamic_cast<ServerLevel*>(mLevel);
        if (serverLevel != NULL && healthBefore - healthAfter > 2.0f)
        {
            int count = (int)((healthBefore - healthAfter) * 0.5);
            serverLevel->sendParticles(PARTICLE_DAMAGE_INDICATOR, entity->mX, entity->mY + entity->mBbHeight * 0.5f,
                                       entity->mZ, count, 0.1, 0.0, 0.1, 0.2, arrayWithLength<int>());
        }
    }

    causeFoodExhaustion((float)FoodConstants::ExhaustionAttack());
}
