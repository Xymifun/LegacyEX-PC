#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdarg>

#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Mth.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/Team.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/EntityPos.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/EntityDataWalker.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/TrackedEntity.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/item/DataItem.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PortalForcer.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/StringTag.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/ClientboundSetPassengersPacket.hpp"
#include "game/network/ClientboundSetEntityLinkPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/item/ProtectionEnchantment.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/Vec3i.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/FenceBlock.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/PortalBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModViews.hpp"
#endif

class ServerPlayer;

const std::wstring Entity::RIDING_TAG = L"Riding";

static unsigned int gSmallIdTlsIndex = TlsAlloc();

const EntityDataAccessor<int>* Entity::DATA_SHARED_FLAGS_ID;
const EntityDataAccessor<int>* Entity::DATA_AIR_SUPPLY_ID;
const EntityDataAccessor<std::wstring>* Entity::DATA_CUSTOM_NAME;
const EntityDataAccessor<bool>* Entity::DATA_CUSTOM_NAME_VISIBLE;
const EntityDataAccessor<bool>* Entity::DATA_SILENT;
const EntityDataAccessor<bool>* Entity::DATA_NO_GRAVITY;
const EntityDataAccessor<bool>* Entity::DATA_MOVING;
const EntityDataAccessor<bool>* Entity::DATA_NAMED_BY_RESTRICTED_PLAYER;
const EntityDataAccessor<bool>* Entity::DATA_CREATED_ON_HOST;
const EntityDataAccessor<std::wstring>* Entity::DATA_OWNER_UUID;

double Entity::sViewScale = 1.0;
int Entity::sEntityCount = Entity::SMALL_ID_COUNT;
unsigned int Entity::sSmallIdsInUse[Entity::SMALL_ID_COUNT / 32];
unsigned int Entity::sSmallIdsWandering[Entity::SMALL_ID_COUNT / 32];
unsigned int Entity::sSmallIdsToRemove[Entity::SMALL_ID_COUNT / 32];
int Entity::sWanderingCount;
int Entity::sWanderingTick;
int Entity::sWanderingIds[3];

int indexOf(const std::vector<boost::weak_ptr<Entity> >& entities, const boost::shared_ptr<Entity>& entity)
{
    for (unsigned int i = 0; i < entities.size(); ++i)
    {
        if (entities.at(i).lock() == entity)
        {
            return (int)i;
        }
    }

    return -1;
}

void Entity::staticCtor()
{
    DATA_SHARED_FLAGS_ID = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                       EntityDataSerializers::INT);
    DATA_AIR_SUPPLY_ID = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                     EntityDataSerializers::INT);
    DATA_CUSTOM_NAME = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                   EntityDataSerializers::STRING);
    DATA_CUSTOM_NAME_VISIBLE = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                           EntityDataSerializers::BOOLEAN);
    DATA_SILENT = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_NO_GRAVITY = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                  EntityDataSerializers::BOOLEAN);
    DATA_MOVING = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY, EntityDataSerializers::BOOLEAN);
    DATA_OWNER_UUID = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                  EntityDataSerializers::STRING);
    DATA_CREATED_ON_HOST = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                       EntityDataSerializers::BOOLEAN);
    DATA_NAMED_BY_RESTRICTED_PLAYER = SynchedEntityData::defineId(EntityType::ENTITY, EntityType::ENTITY,
                                                                  EntityDataSerializers::BOOLEAN);
}

void Entity::useSmallIds()
{
    TlsSetValue(gSmallIdTlsIndex, (void*)1);
}

int Entity::getSmallId()
{
    bool skipRemoved = false;

    if (TlsGetValue(gSmallIdTlsIndex) != NULL && gMinecraftServer != NULL)
    {
        skipRemoved = gMinecraftServer->flagEntitiesToBeRemoved(sSmallIdsToRemove);
    }

    for (int word = 0, id = 0; word < SMALL_ID_COUNT / 32; word++, id += 32)
    {
        if (sSmallIdsInUse[word] == 0xFFFFFFFF)
        {
            continue;
        }

        for (int bit = 0; bit < 32; bit++)
        {
            unsigned int mask = 0x80000000U >> bit;

            if (skipRemoved && (sSmallIdsToRemove[word] & mask) != 0)
            {
                continue;
            }

            if ((sSmallIdsInUse[word] & mask) == 0)
            {
                sSmallIdsInUse[word] |= mask;
                return id + bit;
            }
        }
    }

    return -1;
}

void Entity::freeSmallId(int smallId)
{
    if (TlsGetValue(gSmallIdTlsIndex) == NULL || smallId >= SMALL_ID_COUNT || gMinecraftServer == NULL)
    {
        return;
    }

    if (!gMinecraftServer->shuttingDown && mLevel != NULL && mLevel->mIsClientSide)
    {
        return;
    }

    int word = smallId / 32;
    unsigned int mask = 0x80000000U >> (smallId % 32);

    sSmallIdsInUse[word] &= ~mask;
    sSmallIdsWandering[word] &= ~mask;
}

void Entity::fjDebugCheckSmallIdLeaks()
{
}

void Entity::tickExtraWandering()
{
    sWanderingTick++;

    if (sWanderingTick % 600 != 0)
    {
        return;
    }

    int smallId = 0;

    if (sWanderingCount != 0)
    {
        smallId = (sWanderingIds[sWanderingCount - 1] + 1) % SMALL_ID_COUNT;
    }

    sWanderingCount = 0;

    for (int i = 0; i < SMALL_ID_COUNT && sWanderingCount < 3; i++)
    {
        if ((sSmallIdsWandering[smallId / 32] & (0x80000000U >> (smallId % 32))) != 0)
        {
            sWanderingIds[sWanderingCount] = smallId;
            sWanderingCount++;
        }

        smallId = (smallId + 1) % SMALL_ID_COUNT;
    }
}

void Entity::considerForExtraWandering(bool consider)
{
    if (TlsGetValue(gSmallIdTlsIndex) == NULL)
    {
        return;
    }

    int smallId = getId();

    if (smallId >= SMALL_ID_COUNT)
    {
        return;
    }

    unsigned int mask = 0x80000000U >> (smallId % 32);

    if (consider)
    {
        sSmallIdsWandering[smallId / 32] |= mask;
    }
    else
    {
        sSmallIdsWandering[smallId / 32] &= ~mask;
    }
}

bool Entity::isExtraWanderingEnabled()
{
    if (TlsGetValue(gSmallIdTlsIndex) == NULL)
    {
        return false;
    }

    int smallId = getId();

    if (smallId >= SMALL_ID_COUNT)
    {
        return false;
    }

    for (int i = 0; i < sWanderingCount; i++)
    {
        if (sWanderingIds[i] == smallId)
        {
            return true;
        }
    }

    return false;
}

int Entity::getWanderingQuadrant()
{
    return (getId() + sWanderingTick / 600) & 3;
}

Entity::Entity(Level* level, bool createEntityData) : mPistonDeltas(NULL), mPistonDeltaCount(0)
{
    mLevel = level;

    MemSect(16);
    _init(createEntityData, level);
    MemSect(0);

    setPos(0.0, 0.0, 0.0);

    if (level != NULL)
    {
        mDimension = static_cast<char>(level->mDimension->getType()->getId());
    }

    if (createEntityData)
    {
        mEntityData = boost::shared_ptr<SynchedEntityData>(new SynchedEntityData(this));
    }
    else
    {
        mEntityData = boost::shared_ptr<SynchedEntityData>();
    }

    if (mEntityData != NULL)
    {
        mEntityData->createDateItem(DATA_SHARED_FLAGS_ID, 0);
        mEntityData->define<int>(DATA_AIR_SUPPLY_ID, getMaxAirSupply());
        mEntityData->define(DATA_CUSTOM_NAME_VISIBLE, false);
        mEntityData->define(DATA_CUSTOM_NAME, std::wstring(L""));
        mEntityData->define(DATA_SILENT, false);
        mEntityData->define(DATA_NO_GRAVITY, false);
        mEntityData->define(DATA_MOVING, false);
        mEntityData->define(DATA_NAMED_BY_RESTRICTED_PLAYER, false);
        mEntityData->define(DATA_CREATED_ON_HOST, false);
        mEntityData->define(DATA_OWNER_UUID, std::wstring(L""));
    }

    mCategories = 0;
    mWaterCheckBox = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mFireCacheResult = false;
    mFireCacheMinX = 0;
    mFireCacheMinY = 0;
    mFireCacheMinZ = 0;
    mFireCacheMaxX = 0;
    mFireCacheMaxY = 0;
    mFireCacheMaxZ = 0;
    mLavaCacheResult = false;
    mLavaCacheMinX = 0;
    mLavaCacheMinY = 0;
    mLavaCacheMinZ = 0;
    mLavaCacheMaxX = 0;
    mLavaCacheMaxY = 0;
    mLavaCacheMaxZ = 0;
    mInWallCacheResult = false;
    mInWallCacheX = 0.0;
    mInWallCacheY = 0.0;
    mInWallCacheZ = 0.0;
    mInWallCacheYRot = 0.0f;
    mInWallCacheXRot = 0.0f;
    mAutoSpinAttackTicks = 0;
}

void Entity::_init(bool createEntityData, Level* level)
{
    if (createEntityData && TlsGetValue(gSmallIdTlsIndex) != NULL)
    {
        mId = getSmallId();
    }
    else
    {
        mId = sEntityCount;
        sEntityCount++;

        if (sEntityCount == 0x7FFFFFF)
        {
            sEntityCount = SMALL_ID_COUNT;
        }
    }

    sViewScale = 1.0;
    mBlocksBuilding = false;

    mVehicle.reset();
    mForcedLoading = false;

    mXo = 0.0;
    mYo = 0.0;
    mZo = 0.0;
    mX = 0.0;
    mY = 0.0;
    mZ = 0.0;
    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mYRot = 0.0f;
    mXRot = 0.0f;
    mYRotO = 0.0f;
    mXRotO = 0.0f;
    mBoardingCooldown = 0;
    mVehicleXRotDelta = 0.0;
    mVehicleYRotDelta = 0.0;
    mBoundingBox = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    mOnGround = false;
    mHorizontalCollision = false;
    mVerticalCollision = false;
    mCollision = false;
    mHurtMarked = false;
    mIsInWeb = false;
    mBlocksSpawning = true;
    mRemoved = false;
    mBbWidth = 0.6f;
    mBbHeight = 1.8f;
    mWalkDistO = 0.0f;
    mWalkDist = 0.0f;
    mMoveDist = 0.0f;
    mFallDistance = 0.0f;
    mFallTicks = 0;
    mFallCount = 0;
    mNextStep = 1;
    mNextFlap = 1.0f;
    mXOld = 0.0;
    mYOld = 0.0;
    mZOld = 0.0;
    mMaxUpStep = 0.0f;
    mNoPhysics = false;
    mPushthrough = 0.0f;
    mRandom = new Random();
    mTickCount = 0;
    mIsInWater = false;
    mHeadInWater = false;
    mInvulnerableTime = 0;
    mFirstTick = true;
    mFireImmune = false;
    mInChunk = false;
    mChunkX = 0;
    mChunkY = 0;
    mChunkZ = 0;
    mXp = 0;
    mYp = 0;
    mZp = 0;
    mXRotp = 0;
    mYRotp = 0;
    mNoCulling = false;
    mHasImpulse = false;
    mIsInsidePortal = false;
    mPortalTime = 0;
    mPortalCooldown = 0;
    mDimension = 0;
    mPortalEntranceOffset = NULL;
    mPortalEntranceForwards = NULL;
    mGlowing = false;
    mIgnoreVerticalCollision = false;
    mIgnorePortals = false;

    if (createEntityData)
    {
        std::wstring uuid = Mth::createInsecureUUID(mRandom);
        mUUID = std::wstring(L"ent") + uuid;
    }

    mCommandStats = new CommandStats();
    mRemainingFireTicks = -1;

    mPistonDeltas = new double[3];
    mPistonDeltas[0] = 0.0;
    mPistonDeltas[1] = 0.0;
    mPistonDeltas[2] = 0.0;
    mAllSlots = NonNullList<not_null_ptr<ItemInstance> >();
    mPistonDeltaCount = 3;
    mPistonDeltasGameTime = 0;

    mLeashed = false;
    mLeashInfoTag = NULL;
    mType = 0;
    mShouldTickCounter = 0;

    mWasOnFire = false;
    mFireTransitionStartTick = -1;

    mEyeHeight = getEyeHeight();
    mEyeHeightO = mEyeHeight;
    mSwimAmount = 0.0f;
    mSwimAmountO = 0.0f;
}

Entity::~Entity()
{
    freeSmallId(mId);

    delete mRandom;
    delete mBoundingBox;
    delete mCommandStats;
    delete[] mAllSlots.items.data;
    delete[] mPistonDeltas;
    delete mWaterCheckBox;

    delete mLeashInfoTag;
    mLeashInfoTag = NULL;
}

void Entity::fjDerivedCtorCalls()
{
    mType = GetType();

    defineSynchedData();
}

void Entity::fjCheckDerivedConstruction()
{
}

int Entity::getId()
{
    return mId;
}

void Entity::setId(int id)
{
    mId = id;
}

boost::shared_ptr<SynchedEntityData> Entity::getEntityData()
{
    return mEntityData;
}

bool Entity::hasCategory(EntityCategory::E_EntityCategory category) const
{
    return (mCategories & category) == category;
}

void Entity::sendMessage(ClientboundChatPacket::EChatPacketMessage message, int* intParams, unsigned int numIntParams,
                         std::wstring* stringParams, unsigned int numStringParams)
{
}

bool Entity::hasPermission(EGameCommand command)
{
    return true;
}

std::wstring Entity::getDisplayName()
{
    return getName();
}

BlockPos Entity::getCommandSenderBlockPosition()
{
    return BlockPos(mX, mY + 0.5, mZ);
}

Vec3* Entity::getCommandSenderWorldPosition()
{
    return Vec3::newTemp(mX, mY, mZ);
}

Level* Entity::getCommandSenderWorld()
{
    return mLevel;
}

boost::shared_ptr<Entity> Entity::getCommandSenderEntity()
{
    return shared_from_this();
}

bool Entity::isSendMessageEnabled()
{
    return false;
}

void Entity::setCommandStats(CommandStats::CommandStatType statType, int value)
{
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        mCommandStats->setCommandStats(mLevel->getServer(),
                                       boost::dynamic_pointer_cast<CommandSender>(shared_from_this()), statType, value);
    }
}

MinecraftServer* Entity::getServer()
{
    return mLevel->getServer();
}

void Entity::kill()
{
    remove();
}

void Entity::remove()
{
    mRemoved = true;

    stopCurrentLerp();
}

void Entity::setDropContainerContent(bool dropContainerContent)
{
}

void Entity::resetPos()
{
    if (mLevel == NULL)
    {
        return;
    }

    while (mY > 0.0 && mY < 256.0)
    {
        setPos(mX, mY, mZ);

        if (mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox, false, false, false)->empty())
        {
            break;
        }

        mY += 1.0;
    }

    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mXRot = 0.0f;
}

void Entity::setSize(float width, float height)
{
    if (width == mBbWidth && height == mBbHeight)
    {
        return;
    }

    float oldWidth = mBbWidth;

    mBbWidth = width;
    mBbHeight = height;

    if (mBbWidth < oldWidth)
    {
        double halfWidth = (double)width * 0.5;

        setBoundingBox(
            AABB::newTemp(mX - halfWidth, mY, mZ - halfWidth, mX + halfWidth, mY + (double)height, mZ + halfWidth));
        return;
    }

    AABB* box = mBoundingBox;

    setBoundingBox(AABB::newTemp(box->mMinX, box->mMinY, box->mMinZ, box->mMinX + (double)mBbWidth,
                                 box->mMinY + (double)mBbHeight, box->mMinZ + (double)mBbWidth));

    if (oldWidth < mBbWidth && !mFirstTick && !mLevel->mIsClientSide)
    {
        move(eMoverType_Self, (double)(oldWidth - mBbWidth), 0.0, (double)(oldWidth - mBbWidth), false);
    }
}

void Entity::setPos(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;

    float halfWidth = mBbWidth / 2.0f;
    float height = mBbHeight;

    setBoundingBox(AABB::newTemp(x - halfWidth, y, z - halfWidth, x + halfWidth, y + height, z + halfWidth));
}

void Entity::setRot(float yRot, float xRot)
{
    yRot = Mth::wrapDegrees(yRot);
    xRot = Mth::wrapDegrees(xRot);

    mYRot = yRot;
    mXRot = xRot;
}

void Entity::setPos(EntityPos* pos)
{
    if (pos->mHasPos)
    {
        setPos(pos->mX, pos->mY, pos->mZ);
    }
    else
    {
        setPos(mX, mY, mZ);
    }

    if (pos->mHasRot)
    {
        setRot(pos->mYRot, pos->mXRot);
    }
    else
    {
        setRot(mYRot, mXRot);
    }
}

void Entity::turn(float yRotDelta, float xRotDelta)
{
    float oldYRot = mYRot;
    float oldXRot = mXRot;

    mXRot = -(xRotDelta * 0.15f - mXRot);
    mYRot = yRotDelta * 0.15f + mYRot;
    mXRot = Mth::clamp(mXRot, -90.0f, 90.0f);

    mXRotO += mXRot - oldXRot;
    mYRotO += mYRot - oldYRot;

    if (mVehicle)
    {
        mVehicle->onPassengerTurned(shared_from_this());
    }
}

void Entity::interpolateTurn(float yRotDelta, float xRotDelta)
{
    mXRot = -(xRotDelta * 0.15f - mXRot);
    mYRot = yRotDelta * 0.15f + mYRot;
    mXRot = Mth::clamp(mXRot, -90.0f, 90.0f);
}

void Entity::DamageNearbyMobs()
{
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(shared_from_this(),
                                                                            mBoundingBox->grow(1.0, 1.0, 1.0), NULL);

    if (!entities->empty())
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            DoAutoAttackOnTouch(*it);

            mXd = -(mXd * 0.20000000298023224);
            mYd = -(mYd * 0.20000000298023224);
            mZd = -(mZd * 0.20000000298023224);
            mFallDistance = 0.0f;

            setDamageNearbyMobs(false);
            stopSpinAttack();
        }
    }
    else if (mHorizontalCollision)
    {
        setDamageNearbyMobs(false);
        stopSpinAttack();
    }
}

void Entity::tick()
{
    if (!mLevel->mIsClientSide)
    {
        setSharedFlag(FLAG_GLOWING, isGlowing());
    }

    if (mLevel->mIsClientSide && getDamageNearbyMobs())
    {
        DamageNearbyMobs();

        mAutoSpinAttackTicks++;

        if (mAutoSpinAttackTicks >= 20 || (mAutoSpinAttackTicks > 5 && mOnGround))
        {
            setDamageNearbyMobs(false);
            stopSpinAttack();
        }
    }
    else
    {
        mAutoSpinAttackTicks = 0;
    }

    PIXBeginNamedEvent(0.0f, "Entity tick");
    baseTick();
    PIXEndNamedEvent();
}

bool Entity::isInWaterOrRain()
{
    if (mIsInWater)
    {
        return true;
    }

    if (!mLevel->isRaining())
    {
        return false;
    }

    if (mLevel->isRainingAt(BlockPos(mX, mY, mZ)))
    {
        return true;
    }

    return mLevel->isRainingAt(BlockPos(mX, mY + (double)mBbHeight, mZ));
}

bool Entity::isInWaterRainOrBubble()
{
    BlockPos pos(mX, mY, mZ);
    if (mLevel->getBlockId(pos.x, pos.y, pos.z) == BlockId::BUBBLE_COLUMN_SEGMENT)
        return true;
    return isInWaterOrRain();
}

bool Entity::isInWaterOrBubble()
{
    BlockPos pos(mX, mY, mZ);
    if (mLevel->getBlockId(pos.x, pos.y, pos.z) == BlockId::BUBBLE_COLUMN_SEGMENT)
        return true;
    return isInWater();
}

bool Entity::isInLava()
{
    return mLevel->isInLava(this, mBoundingBox->grow(-0.10000000149011612, -0.4000000059604645, -0.10000000149011612));
}

void Entity::lavaHurt()
{
    if (mFireImmune)
    {
        return;
    }

    hurt(DamageSource::LAVA, 4.0f);
    setSecondsOnFire(15);
}

void Entity::baseTick()
{
    if (isPassenger() && getVehicle()->mRemoved)
    {
        stopRiding();
    }

    if (mBoardingCooldown > 0)
    {
        mBoardingCooldown--;
    }

    mWalkDistO = mWalkDist;
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    mYRotO = mYRot;
    mXRotO = mXRot;

    if (!mLevel->mIsClientSide)
    {
        bool portalsEnabled = true;

        if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        {
            portalsEnabled = !Minecraft::GetInstance()->GetMiniGame()->ArePortalsDisabled();
        }

        if (!mIgnorePortals && portalsEnabled)
        {
            if (mIsInsidePortal)
            {
                if (dynamic_cast<ServerLevel*>(mLevel)->getServer()->isNetherEnabled())
                {
                    if (!isPassenger())
                    {
                        int waitTime = getPortalWaitTime();

                        if (mPortalTime++ >= waitTime)
                        {
                            mPortalTime = waitTime;
                            mPortalCooldown = getDimensionChangingDelay();

                            int dimension = -1;

                            if (mLevel->mDimension->getType() == DimensionType::NETHER)
                            {
                                dimension = 0;
                            }

                            setSprinting(false);
                            changeDimension(dimension);
                        }
                    }

                    mIsInsidePortal = false;
                }
            }
            else
            {
                if (mPortalTime > 0)
                {
                    mPortalTime -= 4;
                }

                if (mPortalTime < 0)
                {
                    mPortalTime = 0;
                }
            }

            processDimensionDelay();
        }
    }

    updateSprintingState();
    updateInWaterState();

    mSwimAmountO = mSwimAmount;

    if (isSwimming())
    {
        mSwimAmount = std::min(1.0f, mSwimAmount + 0.1f);
    }
    else
    {
        mSwimAmount = std::max(0.0f, mSwimAmount - 0.1f);
    }

    if (mFirstTick)
    {
        mWasOnFire = isOnFire();

        if (mWasOnFire)
        {
            mFireTransitionStartTick = 0;
        }
    }
    else if (isOnFire() != mWasOnFire)
    {
        mWasOnFire = isOnFire();
        mFireTransitionStartTick = mTickCount;
    }

    if (mLevel->mIsClientSide)
    {
        clearFire();
    }
    else if (mRemainingFireTicks > 0)
    {
        if (mFireImmune)
        {
            mRemainingFireTicks -= 4;

            if (mRemainingFireTicks < 0)
            {
                clearFire();
            }
        }
        else
        {
            if (mRemainingFireTicks % 20 == 0)
            {
                hurt(DamageSource::ON_FIRE, 1.0f);
            }

            mRemainingFireTicks--;
        }
    }

    PIXBeginNamedEvent(0.0f, "Check for fire");

    bool inWater = isInWaterRainOrBubble();

    if (mLevel->containsFireBlock(this, mBoundingBox->shrink(0.001, 0.001, 0.001)))
    {
        burn(1);

        if (!inWater && ++mRemainingFireTicks == 0)
        {
            setSecondsOnFire(8);
        }
    }
    else if (mRemainingFireTicks < 1)
    {
        mRemainingFireTicks = -1;
    }

    PIXEndNamedEvent();

    if (inWater && isOnFire())
    {
        PIXBeginNamedEvent(0.0f, "Play fizz sound");
        playSound(SoundEvents::GENERIC_EXTINGUISH_FIRE, 0.7f,
                  (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4f + 1.6f);
        mRemainingFireTicks = -1;
        PIXEndNamedEvent();
    }

    if (isInLava())
    {
        lavaHurt();
        mFallDistance *= 0.5f;
    }

    if (mY < -64.0)
    {
        outOfWorld();
    }

    if (!mLevel->mIsClientSide)
    {
        setSharedFlag(FLAG_ONFIRE, mRemainingFireTicks > 0);
    }

    mFirstTick = false;
}

void Entity::processDimensionDelay()
{
    if (mPortalCooldown > 0)
    {
        mPortalCooldown--;
    }
}

int Entity::getPortalWaitTime()
{
    return 1;
}

void Entity::setSecondsOnFire(int seconds)
{
    int ticks = seconds * 20;

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, GetType()))
    {
        ticks = ProtectionEnchantment::getFireAfterDampener(
            boost::static_pointer_cast<LivingEntity>(shared_from_this()), ticks);
    }

    if (mRemainingFireTicks < ticks)
    {
        mRemainingFireTicks = ticks;
    }
}

void Entity::clearFire()
{
    mRemainingFireTicks = 0;
}

void Entity::outOfWorld()
{
    remove();
}

bool Entity::isFree(AABB* box)
{
    return mLevel->getCollisionAABBs(shared_from_this(), box, false, false, false)->empty()
           && !mLevel->containsAnyLiquid(box);
}

bool Entity::isFree(double x, double y, double z)
{
    return isFree(mBoundingBox->move(x, y, z));
}

std::vector<boost::weak_ptr<Entity> >* Entity::getPassengers()
{
    return &mPassengers;
}

void Entity::move(MoverType type, double x, double y, double z, bool pushedByPiston)
{
    std::vector<boost::weak_ptr<Entity> > passengers = *getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers.begin(); it != passengers.end(); ++it)
    {
        boost::shared_ptr<Entity> passenger = it->lock();

        if (passenger && eTYPE_DERIVED_FROM(EntityType::LOCAL_PLAYER, passenger->GetType()))
        {
            if (!boost::static_pointer_cast<LocalPlayer>(passenger)->canMoveOrBeMovedFromTo(mX, mY, mZ, mX + x, mY + y,
                                                                                            mZ + z, NULL))
            {
                return;
            }
        }
    }

    if (mNoPhysics)
    {
        setBoundingBox(mBoundingBox->move(x, y, z));
        setLocationFromBoundingbox();
        return;
    }

    if (type == eMoverType_Piston)
    {
        long long gameTime = mLevel->getGameTime();

        if (gameTime != mPistonDeltasGameTime)
        {
            delete[] mPistonDeltas;
            mPistonDeltas = new double[3];
            mPistonDeltas[0] = 0.0;
            mPistonDeltas[1] = 0.0;
            mPistonDeltas[2] = 0.0;
            mPistonDeltaCount = 3;
            mPistonDeltasGameTime = gameTime;
        }

        if (x != 0.0)
        {
            int axis = Direction::Axis::X->getId();
            double moved = Mth::clamp(x + mPistonDeltas[axis], -0.51, 0.51);

            x = moved - mPistonDeltas[axis];
            mPistonDeltas[axis] = moved;

            if (fabs(x) <= 2.220446049250313e-16)
            {
                return;
            }
        }
        else if (y != 0.0)
        {
            int axis = Direction::Axis::Y->getId();
            double moved = Mth::clamp(y + mPistonDeltas[axis], -0.51, 0.51);

            y = moved - mPistonDeltas[axis];
            mPistonDeltas[axis] = moved;

            if (fabs(y) <= 2.220446049250313e-16)
            {
                return;
            }
        }
        else if (z != 0.0)
        {
            int axis = Direction::Axis::Z->getId();
            double moved = Mth::clamp(z + mPistonDeltas[axis], -0.51, 0.51);

            z = moved - mPistonDeltas[axis];
            mPistonDeltas[axis] = moved;

            if (fabs(z) <= 2.220446049250313e-16)
            {
                return;
            }
        }
        else
        {
            return;
        }
    }

    double startX = mX;
    double startY = mY;
    double startZ = mZ;

    if (mIsInWeb)
    {
        mIsInWeb = false;

        x *= 0.25;
        y *= 0.05000000074505806;
        z *= 0.25;

        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
    }

    double wantedX = x;
    double wantedY = y;
    double wantedZ = z;

    if ((type == eMoverType_Self || type == eMoverType_Player) && mOnGround && isSneaking()
        && eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()))
    {
        while (x != 0.0
               && mLevel
                      ->getCollisionAABBs(shared_from_this(), mBoundingBox->move(x, -(double)mMaxUpStep, 0.0), false,
                                          false, false)
                      ->empty())
        {
            if (x < 0.05 && x >= -0.05)
            {
                x = 0.0;
            }
            else if (x > 0.0)
            {
                x -= 0.05;
            }
            else
            {
                x += 0.05;
            }

            wantedX = x;
        }

        while (z != 0.0
               && mLevel
                      ->getCollisionAABBs(shared_from_this(), mBoundingBox->move(0.0, -(double)mMaxUpStep, z), false,
                                          false, false)
                      ->empty())
        {
            if (z < 0.05 && z >= -0.05)
            {
                z = 0.0;
            }
            else if (z > 0.0)
            {
                z -= 0.05;
            }
            else
            {
                z += 0.05;
            }

            wantedZ = z;
        }

        while (x != 0.0 && z != 0.0
               && mLevel
                      ->getCollisionAABBs(shared_from_this(), mBoundingBox->move(x, -(double)mMaxUpStep, z), false,
                                          false, false)
                      ->empty())
        {
            if (x < 0.05 && x >= -0.05)
            {
                x = 0.0;
            }
            else if (x > 0.0)
            {
                x -= 0.05;
            }
            else
            {
                x += 0.05;
            }

            wantedX = x;

            if (z < 0.05 && z >= -0.05)
            {
                z = 0.0;
            }
            else if (z > 0.0)
            {
                z -= 0.05;
            }
            else
            {
                z += 0.05;
            }

            wantedZ = z;
        }
    }

    PIXBeginNamedEvent(0.0f, "Get cubes");

    std::vector<AABB*>* cubes = mLevel->getCollisionAABBs(
        shared_from_this(), mBoundingBox->expand(x, y, z)->expand(0.0, -y, 0.0)->expand(0.0, (double)mMaxUpStep, 0.0),
        pushedByPiston, true, false);

    PIXEndNamedEvent();

    AABB startBox;
    startBox = *mBoundingBox;

    int chunkX = Mth::floor(mX * 0.0625);
    int chunkZ = Mth::floor(mZ * 0.0625);

    bool sleeping = false;

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()))
    {
        sleeping = boost::static_pointer_cast<Player>(shared_from_this())->isSleeping();
    }

    if (!sleeping)
    {
        if (y != 0.0 && (!mLevel->mIsClientSide || mLevel->reallyHasChunk(chunkX, chunkZ)))
        {
            PIXBeginNamedEvent(0.0f, "Clipy");

            int i = 0;
            int count = (int)cubes->size();

            while (y != 0.0)
            {
                if (i >= count)
                {
                    setBoundingBox(mBoundingBox->move(0.0, y, 0.0));
                    break;
                }

                y = cubes->at(i)->clipYCollide(mBoundingBox, y);
                i++;
            }

            PIXEndNamedEvent();
        }

        PIXBeginNamedEvent(0.0f, "ClipX");

        if (x != 0.0)
        {
            int i = 0;
            int count = (int)cubes->size();

            while (x != 0.0)
            {
                if (i >= count)
                {
                    setBoundingBox(mBoundingBox->move(x, 0.0, 0.0));
                    break;
                }

                x = cubes->at(i)->clipXCollide(mBoundingBox, x);
                i++;
            }
        }

        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "ClipZ");

        if (z != 0.0)
        {
            int i = 0;
            int count = (int)cubes->size();

            while (z != 0.0)
            {
                if (i >= count)
                {
                    setBoundingBox(mBoundingBox->move(0.0, 0.0, z));
                    break;
                }

                z = cubes->at(i)->clipZCollide(mBoundingBox, z);
                i++;
            }
        }

        PIXEndNamedEvent();
    }

    bool steppedOnGround = mOnGround || (wantedY != y && wantedY < 0.0);

    PIXBeginNamedEvent(0.0f, "Step up");

    double clippedX = x;
    double clippedY = y;
    double clippedZ = z;

    if (mMaxUpStep > 0.0f && steppedOnGround && (wantedX != x || wantedZ != z))
    {
        AABB clippedBox;
        clippedBox = *mBoundingBox;

        setBoundingBox(&startBox);

        y = (double)mMaxUpStep;
        z = wantedZ;
        x = wantedX;

        AABB* stepBox = mBoundingBox->expand(wantedX, 0.0, wantedZ);

        double stepY = y;
        AABB* stepped = mBoundingBox;

        if (!mLevel->mIsClientSide || mLevel->reallyHasChunk(chunkX, chunkZ))
        {
            for (std::vector<AABB*>::iterator it = cubes->begin(); stepY != 0.0; ++it)
            {
                if (it == cubes->end())
                {
                    stepped = stepped->move(0.0, stepY, 0.0);
                    break;
                }

                stepY = (*it)->clipYCollide(stepBox, stepY);
            }
        }

        double steppedX = x;

        for (std::vector<AABB*>::iterator it = cubes->begin(); steppedX != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                stepped = stepped->move(steppedX, 0.0, 0.0);
                break;
            }

            steppedX = (*it)->clipXCollide(stepped, steppedX);
        }

        double steppedZ = z;

        for (std::vector<AABB*>::iterator it = cubes->begin(); steppedZ != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                stepped = stepped->move(0.0, 0.0, steppedZ);
                break;
            }

            steppedZ = (*it)->clipZCollide(stepped, steppedZ);
        }

        AABB* raised = mBoundingBox;

        double raisedY = y;

        if (!mLevel->mIsClientSide || mLevel->reallyHasChunk(chunkX, chunkZ))
        {
            for (std::vector<AABB*>::iterator it = cubes->begin(); raisedY != 0.0; ++it)
            {
                if (it == cubes->end())
                {
                    raised = raised->move(0.0, raisedY, 0.0);
                    break;
                }

                raisedY = (*it)->clipYCollide(raised, raisedY);
            }
        }

        double raisedX = x;

        for (std::vector<AABB*>::iterator it = cubes->begin(); raisedX != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                raised = raised->move(raisedX, 0.0, 0.0);
                break;
            }

            raisedX = (*it)->clipXCollide(raised, raisedX);
        }

        double raisedZ = z;

        for (std::vector<AABB*>::iterator it = cubes->begin(); raisedZ != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                raised = raised->move(0.0, 0.0, raisedZ);
                break;
            }

            raisedZ = (*it)->clipZCollide(raised, raisedZ);
        }

        if (steppedX * steppedX + steppedZ * steppedZ <= raisedX * raisedX + raisedZ * raisedZ)
        {
            x = raisedX;
            z = raisedZ;
            y = -raisedY;
            setBoundingBox(raised);
        }
        else
        {
            x = steppedX;
            z = steppedZ;
            y = -stepY;
            setBoundingBox(stepped);
        }

        y = -(double)mMaxUpStep;

        for (std::vector<AABB*>::iterator it = cubes->begin(); y != 0.0; ++it)
        {
            if (it == cubes->end())
            {
                setBoundingBox(mBoundingBox->move(0.0, y, 0.0));
                break;
            }

            y = (*it)->clipYCollide(mBoundingBox, y);
        }

        if (x * x + z * z <= clippedX * clippedX + clippedZ * clippedZ)
        {
            x = clippedX;
            y = clippedY;
            z = clippedZ;
            setBoundingBox(&clippedBox);
        }
    }

    PIXEndNamedEvent();

    setLocationFromBoundingbox();

    mHorizontalCollision = wantedX != x || wantedZ != z;
    mVerticalCollision = !mIgnoreVerticalCollision && wantedY != y;
    mOnGround = mVerticalCollision && wantedY < 0.0;
    mCollision = mHorizontalCollision || mVerticalCollision;

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, GetType()) && mCollision && !gApp.IsAppPaused())
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(shared_from_this());

        if (living->isFallFlying())
        {
            living->takeFallFlyingCollisionDamage();
        }
    }

    int landingX = Mth::floor(mX);
    int landingY = Mth::floor(mY - 0.2f);
    int landingZ = Mth::floor(mZ);

    PIXBeginNamedEvent(0.0f, "Falling check");

    BlockPos landingPos(landingX, landingY, landingZ);
    Block* landingBlock = mLevel->getBlock(landingPos);

    if (landingBlock->mMaterial == Material::AIR)
    {
        Block* below = mLevel->getBlock(landingPos.below());

        if (below != NULL
            && (dynamic_cast<FenceBlock*>(below) != NULL || dynamic_cast<WallBlock*>(below) != NULL
                || dynamic_cast<FenceGateBlock*>(below) != NULL))
        {
            landingPos = landingPos.below();
            landingBlock = below;
        }
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Check fall damage");
    checkFallDamage(y, mOnGround, landingBlock, landingPos);
    PIXEndNamedEvent();

    updateDeltaAfterMove(landingBlock, wantedX, wantedY, wantedZ, x, y, z);

    if (makeStepSound() && (!mOnGround || !isSneaking() || (GetType() & EntityType::PLAYER) != EntityType::PLAYER)
        && !isPassenger())
    {
        PIXBeginNamedEvent(0.0f, "Make step sound");

        double walkedX = mX - startX;
        double walkedY = 0.0;
        double walkedZ = mZ - startZ;

        if (landingBlock == Blocks::ladder)
        {
            walkedY = mY - startY;
        }

        if (mOnGround)
        {
            landingBlock->stepOn(mLevel, landingPos, shared_from_this());
        }

        mWalkDist += Mth::sqrt(walkedX * walkedX + walkedZ * walkedZ) * 0.6f;
        mMoveDist += Mth::sqrt(walkedX * walkedX + walkedY * walkedY + walkedZ * walkedZ) * 0.6f;

        if (mMoveDist > (float)mNextStep && landingBlock->mMaterial != Material::AIR)
        {
            mNextStep = (int)mMoveDist + 1;

            if (isInWater())
            {
                playUnderwaterSound(landingPos, landingBlock);
            }
            else
            {
                playStepSound(landingPos, landingBlock);
            }

            PIXEndNamedEvent();
        }
        else
        {
            if (mMoveDist > mNextFlap && makeFlySound() && landingBlock->mMaterial == Material::AIR)
            {
                mNextFlap = playFlySound(mMoveDist);
            }

            PIXEndNamedEvent();
        }
    }

    PIXBeginNamedEvent(0.0f, "checkInsideBlocks");
    checkInsideBlocks();
    PIXEndNamedEvent();
}

void Entity::updateDeltaAfterMove(Block* block, double x, double y, double z, double& movedX, double& movedY,
                                  double& movedZ)
{
    if (x != movedX)
    {
        mXd = 0.0;
    }

    if (z != movedZ)
    {
        mZd = 0.0;
    }

    if (y != movedY)
    {
        PIXBeginNamedEvent(0.0f, "Update entity after falling");
        block->updateEntityAfterFallOn(mLevel, shared_from_this());
        PIXEndNamedEvent();
    }
}

void Entity::setLocationFromBoundingbox()
{
    AABB* box = mBoundingBox;

    mX = (box->mMinX + box->mMaxX) * 0.5;
    mY = box->mMinY;
    mZ = (box->mMinZ + box->mMaxZ) * 0.5;
}

const SoundEvent* Entity::getSwimSound()
{
    return SoundEvents::GENERIC_SWIM;
}

const SoundEvent* Entity::getSwimSplashSound()
{
    return SoundEvents::GENERIC_SPLASH;
}

const SoundEvent* Entity::getHeavySplashSound()
{
    return getSwimSplashSound();
}

void Entity::checkInsideBlocks()
{
    AABB* box = mBoundingBox;

    BlockPos from(box->mMinX + 0.001, box->mMinY + 0.001, box->mMinZ + 0.001);
    BlockPos to(box->mMaxX - 0.001, box->mMaxY - 0.001, box->mMaxZ - 0.001);

    bool inMiniGame = Minecraft::InMiniGame(eMiniGameId_Any, true);

    if (!mLevel->hasChunksAt(from, to))
    {
        return;
    }

    for (int x = from.x; x <= to.x; x++)
    {
        for (int y = from.y; y <= to.y; y++)
        {
            for (int z = from.z; z <= to.z; z++)
            {
                BlockPos pos(x, y, z);
                const BlockState* state = mLevel->getBlockState(mTickCache, pos);

                if (inMiniGame && !gApp.ShouldCollideWith(state->getBlock()))
                {
                    continue;
                }

                state->getBlock()->entityInside(mLevel, pos, state, shared_from_this());
                onInsideBlock(state);
            }
        }
    }
}

void Entity::onInsideBlock(const BlockState* state)
{
}

_SkinAdjustments Entity::getSkinAdjustments()
{
    if (GameSettings::GetGameSettings(eGameSetting_CustomSkinAnim) == 0
        && (mSkinAdjustments.mAnimFlags & 0x1F1F810) == 0)
    {
        return _SkinAdjustments();
    }

    return mSkinAdjustments;
}

void Entity::setSkinAdjustments(_SkinAdjustments skinAdjustments)
{
    mSkinAdjustments = skinAdjustments;
}

void Entity::playStepSound(const BlockPos& pos, Block* block)
{
    const SoundType* soundType = block->getSoundType();

    MemSect(31);

    if (!eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()) || (getSkinAdjustments().mAnimFlags & 0x4) == 0)
    {
        if (mLevel->getBlockState(mTickCache, pos.above())->getBlock() == Blocks::snow_layer)
        {
            soundType = Blocks::snow_layer->getSoundType();

            playSound(soundType->getStepSound(), soundType->getVolume() * 0.15f, soundType->getPitch());
        }
        else if (!block->defaultBlockState()->getMaterial()->isLiquid())
        {
            playSound(soundType->getStepSound(), soundType->getVolume() * 0.15f, soundType->getPitch());
        }

        MemSect(0);
    }
}

void Entity::playUnderwaterSound(const BlockPos& pos, Block* block)
{
    boost::shared_ptr<Entity> source = (isVehicle() && getControllingPassenger()) ? getControllingPassenger()
                                                                                  : shared_from_this();

    float volume = source == shared_from_this() ? 0.55f : 0.6f;

    float speed = (float)sqrt(source->mXd * source->mXd * 0.20000000298023224 + source->mYd * source->mYd
                              + source->mZd * source->mZd * 0.20000000298023224);

    float amount = speed * volume;

    if (amount > 1.0f)
    {
        amount = 1.0f;
    }

    playSound(getSwimSound(), amount, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4f + 1.0f);
}

float Entity::playFlySound(float moveDist)
{
    return 0.0f;
}

bool Entity::makeFlySound()
{
    return false;
}

void Entity::playSoundToAllPlayers(const SoundEvent* sound, float volume, float pitch)
{
    if (isSilent())
    {
        return;
    }

    mLevel->playSoundToAllPlayers(BlockPos(mX, mY, mZ), sound, getSoundSource(), volume, pitch, 16.0f);
}

void Entity::playSound(const SoundEvent* sound, float volume, float pitch)
{
    if (isSilent())
    {
        return;
    }

    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, sound, getSoundSource(), volume, pitch, 16.0f);
}

bool Entity::isSilent()
{
    return mEntityData->get(DATA_SILENT);
}

void Entity::setSilent(bool silent)
{
    mEntityData->set(DATA_SILENT, silent, true);
}

bool Entity::isNoGravity()
{
    return mEntityData->get(DATA_NO_GRAVITY);
}

void Entity::setNoGravity(bool noGravity)
{
    mEntityData->set(DATA_NO_GRAVITY, noGravity, true);
}

bool Entity::isMoving()
{
    return mEntityData->get(DATA_MOVING);
}

void Entity::setMoving(bool moving)
{
    mEntityData->set(DATA_MOVING, moving, true);
}

bool Entity::makeStepSound()
{
    return true;
}

void Entity::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
    if (onGround)
    {
        if (mFallDistance > 0.0f)
        {
            block->fallOn(mLevel, pos, shared_from_this(), mFallDistance);
            mFallCount++;
        }

        mFallDistance = 0.0f;
        mFallTicks = 0;
    }
    else if (y < 0.0)
    {
        mFallTicks++;
        mFallDistance -= (float)y;
    }
}

AABB* Entity::getCollideBox()
{
    return NULL;
}

void Entity::burn(int damage)
{
    if (mFireImmune)
    {
        return;
    }

    hurt(DamageSource::IN_FIRE, (float)damage);
}

bool Entity::isFireImmune()
{
    return mFireImmune;
}

void Entity::causeFallDamage(float fallDistance, float damageMultiplier)
{
    if (!isVehicle())
    {
        return;
    }

    std::vector<boost::weak_ptr<Entity> > passengers = *getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers.begin(); it != passengers.end(); ++it)
    {
        boost::shared_ptr<Entity> passenger = it->lock();

        if (passenger)
        {
            passenger->causeFallDamage(fallDistance, damageMultiplier);
        }
    }
}

bool Entity::isInWater()
{
    return mIsInWater;
}

bool Entity::headInWater()
{
    return mHeadInWater;
}

bool Entity::isOverWater()
{
    return mLevel->checkAndHandleWater(mBoundingBox->grow(0.0, -20.0, 0.0)->shrink(0.001), Material::WATER,
                                       shared_from_this());
}

Vec3* Entity::getEyePosition(float partialTicks)
{
#ifdef MINECRAFT_PC
    double aimEye[3];
    double aimView[3];
    if (PcModViewsAimFor(this, aimEye, aimView))
        return Vec3::newTemp(aimEye[0], aimEye[1], aimEye[2]);
#endif
    if (partialTicks >= 1.0f)
    {
        return Vec3::newTemp(mX, mY + (double)getEyeHeight(), mZ);
    }

    return Vec3::newTemp((mX - mXo) * (double)partialTicks + mXo,
                         (mY - mYo) * (double)partialTicks + mYo + (double)getEyeHeight(),
                         (mZ - mZo) * (double)partialTicks + mZo);
}

bool Entity::updateInWaterState()
{
    mHeadInWater =
        mLevel->getLiquidBlock(BlockPos(getEyePosition(1.0f)))->defaultBlockState()->getMaterial()->isLiquid();

    AABB* box = mBoundingBox->grow(0.0, -0.4000000059604645, 0.0)->shrink(0.001, 0.001, 0.001);

    if (!mIsInWater && !mTickCache.mCacheChanged)
    {
        int minX = Mth::absFloor(box->mMinX);
        int maxX = Mth::ceil(box->mMaxX);
        int minY = Mth::absFloor(box->mMinY);
        int maxY = Mth::ceil(box->mMaxY);
        int minZ = Mth::absFloor(box->mMinZ);
        int maxZ = Mth::ceil(box->mMaxZ);

        if (Mth::absFloor(mWaterCheckBox->mMinX) == minX && Mth::ceil(mWaterCheckBox->mMaxX) == maxX
            && Mth::absFloor(mWaterCheckBox->mMinY) == minY && Mth::ceil(mWaterCheckBox->mMaxY) == maxY
            && Mth::absFloor(mWaterCheckBox->mMinZ) == minZ && Mth::ceil(mWaterCheckBox->mMaxZ) == maxZ)
        {
            mWaterCheckBox->set(box);
            return false;
        }
    }

    mWaterCheckBox->set(box);

    bool inBoat = false;

    if (getVehicle() && eTYPE_DERIVED_FROM(EntityType::BOAT, getVehicle()->GetType()))
    {
        inBoat = true;
    }

    if (!inBoat)
    {
        bool inWater = true;

        if (!mLevel->checkAndHandleWater(mBoundingBox->grow(0.0, -0.4000000059604645, 0.0)->shrink(0.001, 0.001, 0.001),
                                         Material::WATER, shared_from_this()))
        {
            if (!isInLiquid(Material::WATER, getAttachPos(ENTITY_LOCATION_FEET, 0.0f)))
            {
                inWater = false;
            }
        }

        if (inWater)
        {
            if (!mIsInWater && !mFirstTick && canCreateParticles())
            {
                doWaterSplashEffect();
                mFallCount++;
            }

            mFallDistance = 0.0f;
            mFallTicks = 0;
            mIsInWater = true;
            clearFire();

            return mIsInWater;
        }
    }

    mIsInWater = false;

    return false;
}

void Entity::doWaterSplashEffect()
{
    boost::shared_ptr<Entity> source =
        isVehicle() && getControllingPassenger() ? getControllingPassenger() : shared_from_this();

    float volume = source == shared_from_this() ? 0.2f : 0.9f;

    float speed = (float)sqrt(source->mXd * source->mXd * 0.2f + source->mYd * source->mYd
                              + source->mZd * source->mZd * 0.2f)
                  * volume;

    MemSect(31);

    if (speed > 1.0f)
    {
        speed = 1.0f;
    }

    if (speed < 0.25f)
    {
        playSound(getSwimSplashSound(), speed + 0.25f, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4f + 1.0f);
    }
    else
    {
        playSound(getHeavySplashSound(), speed + 0.25f, (mRandom->nextFloat() - mRandom->nextFloat()) * 0.4f + 1.0f);
    }

    MemSect(0);

    float surfaceY = (float)Mth::absFloor(mBoundingBox->mMinY);

    for (int i = 0; (float)i < mBbWidth * 20.0f + 1.0f; i++)
    {
        mLevel->addParticle(PARTICLE_BUBBLE, mX + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            (double)(surfaceY + 1.0f), mZ + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            mXd, mYd - (double)(mRandom->nextFloat() * 0.2f), mZd, arrayWithLength<int>());
    }

    for (int i = 0; (float)i < mBbWidth * 20.0f + 1.0f; i++)
    {
        mLevel->addParticle(PARTICLE_SPLASH, mX + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            (double)(surfaceY + 1.0f), mZ + (double)((mRandom->nextFloat() * 2.0f - 1.0f) * mBbWidth),
                            mXd, mYd, mZd, arrayWithLength<int>());
    }
}

void Entity::updateSprintingState()
{
    if (isSprinting() && !isInWater())
    {
        doSprintParticleEffect();
    }
}

void Entity::doSprintParticleEffect()
{
    int x = Mth::floor(mX);
    int y = Mth::floor(mY - 0.20000000298023224);
    int z = Mth::floor(mZ);
    BlockPos pos(x, y, z);
    const BlockState* state = mLevel->getBlockState(mTickCache, pos);

    if (state->getRenderShape() == BLOCK_RENDER_INVISIBLE)
    {
        return;
    }

    int stateId = Block::getIdWithData(state);

    arrayWithLength<int> particleData(&stateId, 1);
    mLevel->addParticle(PARTICLE_BLOCK_CRACK, ((double)mRandom->nextFloat() - 0.5) * mBbWidth + mX,
                        mBoundingBox->mMinY + 0.1, ((double)mRandom->nextFloat() - 0.5) * mBbWidth + mZ, -(mXd * 4.0),
                        1.5, -(mZd * 4.0), particleData);
}

bool Entity::isUnderLiquid(Material* material)
{
    bool onBoat = false;

    if (getVehicle() && eTYPE_DERIVED_FROM(EntityType::BOAT, getVehicle()->GetType()))
    {
        onBoat = true;
    }

    if (!onBoat)
    {
        double eyeY = mY + (double)getEyeHeight();
        BlockPos pos(mX, eyeY, mZ);

        const BlockState* state = mLevel->getBlockState(mTickCache, pos);

        if (state->getMaterial() != material)
        {
            unsigned short element = mLevel->getExtraElement(pos.x, pos.y, pos.z);

            if (element == 0)
            {
                return false;
            }

            state = Block::getStateByElementId(element);
        }

        if (state->getMaterial() == material)
        {
            double surface = (double)((float)(pos.y + 1)
                                      - (float)(LiquidBlock::getHeightFromData(
                                                    state->getBlock()->convertBlockStateToLegacyData(state))
                                                - 0.11111111f));

            bool inside = eyeY < surface;

            if (!inside && eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()))
            {
                return false;
            }

            return inside;
        }
    }

    return false;
}

bool Entity::isInLiquid(Material* material, Vec3* pos)
{
    bool onBoat = false;

    if (getVehicle() && eTYPE_DERIVED_FROM(EntityType::BOAT, getVehicle()->GetType()))
    {
        onBoat = true;
    }

    if (!onBoat)
    {
        BlockPos blockPos(pos->x, pos->y, pos->z);
        const BlockState* state = mLevel->getBlockState(mTickCache, blockPos);

        if (state->getMaterial() != material)
        {
            unsigned short element = mLevel->getExtraElement(blockPos.x, blockPos.y, blockPos.z);

            if (element == 0)
            {
                return false;
            }

            state = Block::getStateByElementId(element);
        }

        if (state->getMaterial() == material)
        {
            double surface = (double)((float)(blockPos.y + 1)
                                      - (float)(LiquidBlock::getHeightFromData(
                                                    state->getBlock()->convertBlockStateToLegacyData(state))
                                                - 0.11111111f));

            bool inside = pos->y < surface;

            if (!inside && eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()))
            {
                return false;
            }

            return inside;
        }
    }

    return false;
}

void Entity::moveRelative(float x, float y, float z, float speed)
{
    float length = x * x + y * y + z * z;

    if (length < 1.0e-4f)
    {
        return;
    }

    length = sqrtf(length);

    if (length < 1.0f)
    {
        length = 1.0f;
    }

    length = speed / length;

    x *= length;
    y *= length;
    z *= length;

    float sin = Mth::sin(mYRot * Mth::DEG_TO_RAD);
    float cos = Mth::cos(mYRot * Mth::DEG_TO_RAD);

    mXd += (double)(x * cos - z * sin);
    mYd += (double)y;
    mZd += (double)(z * cos + x * sin);
}

int Entity::GetLightColor(Entity* entity)
{
    Level* level = entity->mLevel;
    MutableBlockPos pos(Mth::absFloor(entity->mX), 0, Mth::absFloor(entity->mZ));

    if (!level->hasChunkAt(pos))
    {
        return 0;
    }

    pos.setY(Mth::absFloor(entity->mY + (double)entity->getEyeHeight()));

    return level->getLightColor(pos, 0, -1);
}

int Entity::getLightColor()
{
    return GetLightColor(this);
}

float Entity::getBrightness()
{
    MutableBlockPos pos(Mth::absFloor(mX), 0, Mth::absFloor(mZ));

    if (!mLevel->hasChunkAt(pos))
    {
        return 0.0f;
    }

    pos.setY(Mth::absFloor(mY + (double)getEyeHeight()));

    return mLevel->getBrightness(pos);
}

void Entity::setLevel(Level* level)
{
    mLevel = level;
}

void Entity::absMoveTo(double x, double y, double z, float yRot, float xRot)
{
    mLevel->clampWithinLevelBounds(x, z, 0.0);

    mXo = mX = x;
    mYo = mY = y;
    mZo = mZ = z;

    float wrapped = Mth::wrapDegrees(xRot);

    mYRotO = mYRot = yRot;
    mXRotO = mXRot = wrapped;

    double delta = (double)(mYRotO - yRot);

    if (delta < -180.0)
    {
        mYRotO += 360.0f;
    }

    if (delta >= 180.0)
    {
        mYRotO -= 360.0f;
    }

    setPos(mX, mY, mZ);
    setRot(yRot, wrapped);
}

void Entity::absMoveTo(double x, double y, double z)
{
    mLevel->clampWithinLevelBounds(x, z, 0.0);

    mX = x;
    mXo = x;
    mZo = z;
    mYo = y;
    mZ = z;
    mY = y;

    setPos(x, y, z);
}

void Entity::moveTo(const BlockPos& pos, float yRot, float xRot)
{
    moveTo((double)pos.x, (double)pos.y, (double)pos.z + 0.5, yRot, xRot);
}

void Entity::moveTo(double x, double y, double z, float yRot, float xRot)
{
    mXOld = mXo = mX = x;
    mYOld = mYo = mY = y;
    mZOld = mZo = mZ = z;
    mYRot = yRot;
    mXRot = xRot;

    setPos(mX, mY, mZ);
}

float Entity::distanceTo(boost::shared_ptr<Entity> entity) const
{
    float dx = (float)(mX - entity->mX);
    float dy = (float)(mY - entity->mY);
    float dz = (float)(mZ - entity->mZ);

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

double Entity::distanceToSqr(const BlockPos& pos) const
{
    return pos.distSqr(mX, mY, mZ);
}

double Entity::distanceAsIfHitscanned(const BlockPos& pos, const AABB& box) const
{
    Vec3* center = Vec3::newTemp((double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5);

    double distanceSqr = center->distanceToSqr(box.getCenter());
    double radius = (box.mMaxY - box.mMinY) * 0.5 + ((box.mMaxX - box.mMinX) * 0.5 + 1.8);

    return distanceSqr - radius * radius;
}

double Entity::distanceSqrToBlockPosCenter(const BlockPos& pos) const
{
    return pos.distSqrToCenter(mX, mY, mZ);
}

double Entity::distanceTo(double x, double y, double z) const
{
    double dx = mX - x;
    double dy = mY - y;
    double dz = mZ - z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}

double Entity::distanceToSqr(boost::shared_ptr<Entity> entity) const
{
    double dx = mX - entity->mX;
    double dy = mY - entity->mY;
    double dz = mZ - entity->mZ;

    return dx * dx + dy * dy + dz * dz;
}

double Entity::distanceToSqr(double x, double y, double z) const
{
    double dx = mX - x;
    double dy = mY - y;
    double dz = mZ - z;

    return dx * dx + dy * dy + dz * dz;
}

void Entity::playerTouch(boost::shared_ptr<Player> player)
{
}

void Entity::push(boost::shared_ptr<Entity> entity)
{
    if (isPassengerOfSameVehicle(entity))
    {
        return;
    }

    if (entity->mNoPhysics || mNoPhysics)
    {
        return;
    }

    double dx = entity->mX - mX;
    double dz = entity->mZ - mZ;
    double max = Mth::asbMax(dx, dz);

    if (max < 0.009999999776482582)
    {
        return;
    }

    max = sqrt(max);
    dx /= max;
    dz /= max;

    double scale = 1.0 / max;

    if (scale > 1.0)
    {
        scale = 1.0;
    }

    dx *= scale;
    dz *= scale;
    dx *= 0.05000000074505806;
    dz *= 0.05000000074505806;
    dx *= (double)(1.0f - mPushthrough);
    dz *= (double)(1.0f - mPushthrough);

    if (!isVehicle())
    {
        push(-dx, 0.0, -dz);
    }

    if (!entity->isVehicle())
    {
        entity->push(dx, 0.0, dz);
    }
}

void Entity::push(double x, double y, double z)
{
    mXd += x;
    mYd += y;
    mZd += z;
    mHasImpulse = true;
}

int Entity::getFireTransitionStartTick() const
{
    return mFireTransitionStartTick;
}

void Entity::markHurt()
{
    mHurtMarked = true;
}

bool Entity::hurt(const DamageSource* source, float damage)
{
    if (!isInvulnerableTo(source) || source->isBypassInvul())
    {
        markHurt();
    }

    return false;
}

Vec3* Entity::calculateViewVector(float xRot, float yRot)
{
    float yaw = -(yRot * Mth::DEG_TO_RAD) - 3.1415927f;
    float pitch = -(xRot * Mth::DEG_TO_RAD);

    float cosYaw = Mth::cos(yaw);
    float sinYaw = Mth::sin(yaw);
    float cosPitch = Mth::cos(pitch);
    float sinPitch = Mth::sin(pitch);

    return Vec3::newTemp((double)(sinYaw * -cosPitch), (double)sinPitch, (double)(cosYaw * -cosPitch));
}

Vec3* Entity::getViewVector(float partialTicks)
{
#ifdef MINECRAFT_PC
    double aimEye[3];
    double aimView[3];
    if (PcModViewsAimFor(this, aimEye, aimView))
        return Vec3::newTemp(aimView[0], aimView[1], aimView[2]);
#endif
    if (partialTicks == 1.0f)
    {
        return calculateViewVector(mXRot, mYRot);
    }

    float xRot = mXRotO + (mXRot - mXRotO) * partialTicks;
    float yRot = mYRotO + (mYRot - mYRotO) * partialTicks;
    return calculateViewVector(xRot, yRot);
}

bool Entity::isPickable()
{
    return false;
}

bool Entity::isPushable(boost::shared_ptr<Entity> entity)
{
    return false;
}

void Entity::awardKillScore(boost::shared_ptr<Entity> killed, int score, const DamageSource* source)
{
}

bool Entity::shouldRender(double x, double y, double z)
{
    double dx = mX - x;
    double dy = mY - y;
    double dz = mZ - z;

    return shouldRender(dx * dx + dy * dy + dz * dz);
}

bool Entity::shouldRender(double distanceSqr)
{
    return shouldRenderAtSqrDistance(distanceSqr);
}

bool Entity::shouldRenderAtSqrDistance(double distanceSqr)
{
    double size = mBoundingBox->getSize();

    if (size != size)
    {
        size = 1.0;
    }

    double range = size * (48.0 * sViewScale);

    return distanceSqr < range * range;
}

HitResult* Entity::pick(double distance, float partialTicks)
{
    Vec3* from = getEyePosition(partialTicks);
    Vec3* view = getViewVector(partialTicks);
    Vec3* to = from->add(view->x * distance, view->y * distance, view->z * distance);

    bool inMiniGame = false;
    boost::unordered_set<Block*>* alwaysClipBlocks = NULL;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !Minecraft::InMiniGame(eMiniGameId_Lobby, true))
    {
        inMiniGame = true;
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        {
            void* storage = operator new(sizeof(boost::unordered_set<Block*>));
            alwaysClipBlocks = new (storage) boost::unordered_set<Block*>(11);
            alwaysClipBlocks->insert(Blocks::web);
            alwaysClipBlocks->insert(Blocks::vine);
        }
    }
    HitResult* hit = mLevel->clip(from, to, false, inMiniGame, true, inMiniGame, alwaysClipBlocks, false);
    delete alwaysClipBlocks;
    return hit;
}

std::wstring Entity::getEncodeId()
{
    ResourceLocation key = EntityIO::getKey(GetType());

    if (key.getPath().empty())
    {
        return std::wstring(L"");
    }

    return key.toString();
}

ListTag* Entity::newDoubleList(unsigned int count, double first, ...)
{
    ListTag* list = new ListTag(6);

    list->add(new DoubleTag(first));

    va_list args;
    va_start(args, first);

    for (unsigned int i = 1; i < count; i++)
    {
        list->add(new DoubleTag(va_arg(args, double)));
    }

    va_end(args);

    return list;
}

ListTag* Entity::newFloatList(unsigned int count, float first, float second)
{
    ListTag* list = new ListTag(5);

    list->add(new FloatTag(first));
    list->add(new FloatTag(second));

    return list;
}

int Entity::getAirSupply()
{
    return mEntityData->get(DATA_AIR_SUPPLY_ID);
}

int Entity::getMaxAirSupply()
{
    return 300;
}

void Entity::setAirSupply(int airSupply)
{
    mEntityData->set(DATA_AIR_SUPPLY_ID, airSupply, true);
}

bool Entity::getSharedFlag(int flag)
{
    if (!mEntityData)
    {
        return false;
    }

    return ((unsigned int)mEntityData->get(DATA_SHARED_FLAGS_ID) >> flag & 1) != 0;
}

void Entity::setSharedFlag(int flag, bool value)
{
    if (!mEntityData)
    {
        return;
    }

    int flags = mEntityData->get(DATA_SHARED_FLAGS_ID);

    if (value)
    {
        mEntityData->set(DATA_SHARED_FLAGS_ID, flags | (1 << flag), true);
    }
    else
    {
        mEntityData->set(DATA_SHARED_FLAGS_ID, flags & ~(1 << flag), true);
    }
}

CompoundTag* Entity::saveWithoutId(CompoundTag* tag)
{
    tag->put(std::wstring(L"Pos"), newDoubleList(3, mX, mY, mZ));
    tag->put(std::wstring(L"Motion"), newDoubleList(3, mXd, mYd, mZd));
    tag->put(std::wstring(L"Rotation"), newFloatList(2, mYRot, mXRot));
    tag->putFloat(std::wstring(L"FallDistance"), mFallDistance);
    tag->putShort(std::wstring(L"Fire"), (short)mRemainingFireTicks);
    tag->putShort(std::wstring(L"Air"), (short)getAirSupply());
    tag->putBoolean(std::wstring(L"OnGround"), mOnGround);
    tag->putInt(std::wstring(L"Dimension"), mDimension);
    tag->putBoolean(std::wstring(L"Invulnerable"), getSharedFlag(FLAG_INVULNERABLE));
    tag->putInt(std::wstring(L"PortalCooldown"), mPortalCooldown);
    tag->putString(std::wstring(L"UUID"), getUUID());

    if (!getCustomName().empty())
    {
        tag->putString(std::wstring(L"CustomName"), getCustomName());
        tag->putBoolean(std::wstring(L"namedByRestrictedPlayer"), mEntityData->get(DATA_NAMED_BY_RESTRICTED_PLAYER));
        tag->putString(std::wstring(L"ownerUUID"), mEntityData->get(DATA_OWNER_UUID));
        tag->putBoolean(std::wstring(L"createdOnHost"), mEntityData->get(DATA_CREATED_ON_HOST));
    }

    if (isCustomNameVisible())
    {
        tag->putBoolean(std::wstring(L"CustomNameVisible"), isCustomNameVisible());
    }

    if (isSilent())
    {
        tag->putBoolean(std::wstring(L"Silent"), isSilent());
    }

    if (isNoGravity())
    {
        tag->putBoolean(std::wstring(L"NoGravity"), isNoGravity());
    }

    if (mGlowing)
    {
        tag->putBoolean(std::wstring(L"Glowing"), mGlowing);
    }

    if (!mTags.empty())
    {
        ListTag* tags = new ListTag();

        for (std::set<std::wstring>::const_iterator it = mTags.begin(); it != mTags.end(); ++it)
        {
            tags->add(new StringTag(*it));
        }

        tag->put(std::wstring(L"Tags"), tags);
    }

    addAdditonalSaveData(tag);

    if (isVehicle())
    {
        ListTag* passengers = new ListTag();

        std::vector<boost::weak_ptr<Entity> > riders = *getPassengers();

        for (std::vector<boost::weak_ptr<Entity> >::iterator it = riders.begin(); it != riders.end(); ++it)
        {
            boost::shared_ptr<Entity> passenger = it->lock();

            CompoundTag* passengerTag = new CompoundTag();

            if (passenger && passenger->saveAsPassenger(passengerTag))
            {
                passengers->add(passengerTag);
            }
            else
            {
                delete passengerTag;
            }
        }

        if (!passengers->isEmpty())
        {
            tag->put(RIDING_TAG, passengers);
        }
        else
        {
            delete passengers;
        }
    }

    return tag;
}

bool Entity::saveAsPassenger(CompoundTag* tag)
{
    std::wstring id = getEncodeId();

    if (mRemoved || id.empty())
    {
        return false;
    }

    tag->putString(std::wstring(L"id"), id);
    saveWithoutId(tag);

    return true;
}

bool Entity::save(CompoundTag* tag)
{
    std::wstring id = getEncodeId();

    if (mRemoved || id.empty() || isPassenger())
    {
        return false;
    }

    std::set<boost::shared_ptr<Entity> >* players = getIndirectPassengers(EntityType::SERVER_PLAYER);
    size_t playerCount = players->size();
    delete players;

    if (playerCount == 1)
    {
        return false;
    }

    tag->putString(std::wstring(L"id"), id);
    saveWithoutId(tag);

    return true;
}

void Entity::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
    dataFixerUpper->addWalker(DataFixTypes::eDataFixTypes_Entity, new EntityDataWalker());
}

void Entity::load(CompoundTag* tag)
{
    ListTag* pos = tag->fjSafeGetListTag(L"Pos", Tag::TAG_DOUBLE);
    ListTag* motion = tag->fjSafeGetListTag(L"Motion", Tag::TAG_DOUBLE);
    ListTag* rotation = tag->fjSafeGetListTag(L"Rotation", Tag::TAG_FLOAT);

    mXd = motion->getDouble(0);
    mYd = motion->getDouble(1);
    mZd = motion->getDouble(2);

    if (fabs(mXd) > 10.0)
    {
        mXd = 0.0;
    }

    if (fabs(mYd) > 10.0)
    {
        mYd = 0.0;
    }

    if (fabs(mZd) > 10.0)
    {
        mZd = 0.0;
    }

    mX = pos->getDouble(0);
    mY = pos->getDouble(1);
    mZ = pos->getDouble(2);
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    mXo = mX;
    mYo = mY;
    mZo = mZ;

    mYRot = rotation->getFloat(0);
    mXRot = rotation->getFloat(1);
    mYRotO = mYRot;
    mXRotO = mXRot;

    setYHeadRot(mYRot);
    setYBodyRot(mYRot);

    mFallDistance = tag->getFloat(std::wstring(L"FallDistance"));
    mRemainingFireTicks = tag->getShort(std::wstring(L"Fire"));

    setAirSupply(tag->getShort(std::wstring(L"Air")));

    mOnGround = tag->getBoolean(std::wstring(L"OnGround"));

    if (tag->contains(std::wstring(L"Dimension")))
    {
        mDimension = tag->getInt(std::wstring(L"Dimension"));
    }

    setInvulnerable(tag->getBoolean(std::wstring(L"Invulnerable")));

    mPortalCooldown = tag->getInt(std::wstring(L"PortalCooldown"));

    if (tag->contains(std::wstring(L"UUID")))
    {
        mUUID = tag->getString(std::wstring(L"UUID"));
    }

    setPos(mX, mY, mZ);
    setRot(mYRot, mXRot);

    if (tag->contains(std::wstring(L"CustomName"), 8) && !tag->getString(std::wstring(L"CustomName")).empty())
    {
        setCustomName(tag->getString(std::wstring(L"CustomName")), tag->getString(std::wstring(L"ownerUUID")),
                      tag->getBoolean(std::wstring(L"namedByRestrictedPlayer")));

        mEntityData->set(DATA_CREATED_ON_HOST, tag->getBoolean(std::wstring(L"createdOnHost")), true);
    }

    setCustomNameVisible(tag->getBoolean(std::wstring(L"CustomNameVisible")));
    setSilent(tag->getBoolean(std::wstring(L"Silent")));
    setNoGravity(tag->getBoolean(std::wstring(L"NoGravity")));
    setGlowing(tag->getBoolean(std::wstring(L"Glowing")));

    if (tag->contains(std::wstring(L"Tags"), 9))
    {
        mTags.clear();

        ListTag* tags = tag->getList(std::wstring(L"Tags"), 8);

        int count = tags->size();

        if (count > 1024)
        {
            count = 1024;
        }

        for (int i = 0; i < count; i++)
        {
            mTags.insert(tags->getString(i));
        }
    }

    readAdditionalSaveData(tag);

    if (repositionEntityAfterLoad())
    {
        setPos(mX, mY, mZ);
    }
}

bool Entity::shouldTick()
{
    double radius = mBoundingBox->getSize();

    if (radius != radius)
    {
        radius = 1.0;
    }

    radius *= 48.0 * sViewScale;

    boost::shared_ptr<Entity> nearby = mShouldTickNearbyCache.lock();
    bool needsSearch = true;

    if (nearby && distanceToSqr(nearby) <= radius * radius)
    {
        needsSearch = false;
    }

    if (needsSearch)
    {
        nearby = mLevel->getNearestPlayerPredicate(mX, mY, mZ, radius, NULL);
        mShouldTickNearbyCache = nearby;
    }

    mShouldTickCounter++;

    if (!nearby && mShouldTickCounter % 3 != 0)
    {
        return false;
    }

    return true;
}

bool Entity::repositionEntityAfterLoad()
{
    return true;
}

boost::shared_ptr<ItemEntity> Entity::spawnAtLocation(Item* item, int count)
{
    return spawnAtLocation(item, count, 0.0f);
}

boost::shared_ptr<ItemEntity> Entity::spawnAtLocation(Item* item, int count, float yOffset)
{
    return spawnAtLocation(not_null_ptr<ItemInstance>(new ItemInstance(item, count, 0)), yOffset);
}

boost::shared_ptr<ItemEntity> Entity::spawnAtLocation(not_null_ptr<ItemInstance> itemInstance, float yOffset)
{
    if (itemInstance->isEmpty())
    {
        return boost::shared_ptr<ItemEntity>();
    }

    boost::shared_ptr<ItemEntity> itemEntity(new ItemEntity(mLevel, mX, mY + (double)yOffset, mZ, itemInstance));

    itemEntity->setDefaultPickUpDelay();

    mLevel->addEntity(itemEntity);

    return itemEntity;
}

bool Entity::isAlive()
{
    return !mRemoved;
}

bool Entity::isInWall()
{
    if (mNoPhysics)
    {
        return false;
    }

    if (!mTickCache.mCacheChanged && mInWallCacheX == mX && mInWallCacheY == mY && mInWallCacheZ == mZ
        && mInWallCacheYRot == mYRot && mInWallCacheXRot == mXRot)
    {
        return mInWallCacheResult;
    }

    mInWallCacheX = mX;
    mInWallCacheY = mY;
    mInWallCacheZ = mZ;
    mInWallCacheYRot = mYRot;
    mInWallCacheXRot = mXRot;

    BlockPos lastPos((int)0x80000000, (int)0x80000000, (int)0x80000000);

    double eyeHeight = getEyeHeight();

    for (int i = 0; i < 8; i++)
    {
        int x = Mth::floor(mX + (double)(((float)((i / 2) % 2) - 0.5f) * mBbWidth * 0.8f));
        int y = Mth::floor(mY + (double)(((float)(i % 2) - 0.5f) * 0.1f) + (double)eyeHeight);
        int z = Mth::floor(mZ + (double)(((float)((i / 4) % 2) - 0.5f) * mBbWidth * 0.8f));

        BlockPos pos(x, y, z);

        if (lastPos.x == x && lastPos.y == y && lastPos.z == z)
        {
            continue;
        }

        lastPos = pos;

        const BlockState* state = mLevel->getBlockState(mTickCache, pos);

        if (state->isViewBlocking()
            && (!Minecraft::InMiniGame(eMiniGameId_Any, true) || gApp.ShouldCollideWith(state->getBlock())))
        {
            mInWallCacheResult = true;
            return true;
        }
    }

    mInWallCacheResult = false;

    return false;
}

bool Entity::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    return false;
}

AABB* Entity::getCollideAgainstBox(boost::shared_ptr<Entity> entity)
{
    return NULL;
}

void Entity::rideTick()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();

    if (isPassenger() && vehicle->mRemoved)
    {
        stopRiding();
        return;
    }

    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;

    tick();

    if (!isPassenger())
    {
        return;
    }

    vehicle->positionRider(shared_from_this());

    if (eTYPE_DERIVED_FROM(EntityType::MINECART_RIDEABLE, vehicle->GetType()))
    {
        mVehicleYRotDelta += (double)(vehicle->mYRot - vehicle->mYRotO);

        double xRotDelta = mVehicleXRotDelta + (double)(vehicle->mXRot - vehicle->mXRotO);
        double yRotDelta = mVehicleYRotDelta;

        while (yRotDelta >= 180.0)
        {
            yRotDelta -= 360.0;
        }

        while (yRotDelta < -180.0)
        {
            yRotDelta += 360.0;
        }

        while (xRotDelta >= 180.0)
        {
            xRotDelta -= 360.0;
        }

        while (xRotDelta < -180.0)
        {
            xRotDelta += 360.0;
        }

        double yRotStep = yRotDelta * 0.5;
        double xRotStep = xRotDelta * 0.5;

        mVehicleXRotDelta = xRotDelta - xRotStep;
        mVehicleYRotDelta = yRotDelta - yRotStep;

        mYRot += (float)yRotStep;
        mXRot += (float)xRotStep;
    }
}

void Entity::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!hasPassenger(passenger))
    {
        return;
    }

    if (RiderPositionOverrides::OverridePosition(shared_from_this(), passenger))
    {
        return;
    }

    passenger->setPos(mX, mY + getRideHeight() + passenger->getRidingHeight(), mZ);
}

void Entity::onPassengerTurned(boost::shared_ptr<Entity> passenger)
{
}

double Entity::getRidingHeight()
{
    return 0.0;
}

double Entity::getRideHeight()
{
    return (double)mBbHeight * 0.75;
}

bool Entity::showVehicleHealth()
{
    return eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, GetType());
}

bool Entity::startRiding(boost::shared_ptr<Entity> vehicle)
{
    return startRiding(vehicle, false);
}

bool Entity::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    for (boost::shared_ptr<Entity> root = vehicle; root->mVehicle; root = root->mVehicle)
    {
        if (root->mVehicle == shared_from_this())
        {
            return false;
        }
    }

    if (!force && (!canRide(vehicle) || !vehicle->canAddPassenger(shared_from_this())))
    {
        return false;
    }

    mVehicleXRotDelta = 0.0;
    mVehicleYRotDelta = 0.0;

    if (isPassenger())
    {
        stopRiding();
    }

    mVehicle = vehicle;
    mVehicle->addPassenger(shared_from_this());

    return true;
}

unsigned int Entity::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::NAME_TAG
        && tooltipData.m_pItemInstance->hasCustomHoverName()
        && (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName() != getCustomName()))
    {
        return IDS_TOOLTIPS_NAME;
    }

    return IDS_NONE;
}

unsigned int Entity::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToHurt)
    {
        return IDS_NONE;
    }

    return IDS_TOOLTIPS_HIT;
}

bool Entity::HideUseTooltips()
{
    return false;
}

bool Entity::canRide(boost::shared_ptr<Entity> vehicle)
{
    return mBoardingCooldown <= 0;
}

void Entity::ejectPassengers()
{
    for (int i = (int)mPassengers.size() - 1; i >= 0; i--)
    {
        boost::shared_ptr<Entity> passenger = mPassengers.at(i).lock();
        passenger->stopRiding();
    }
}

void Entity::stopRiding()
{
    if (!mVehicle)
    {
        return;
    }

    boost::shared_ptr<Entity> vehicle = mVehicle;

    mVehicle.reset();

    vehicle->removePassenger(shared_from_this());
}

void Entity::fjImmediateBroadcastPassengerPacket(const boost::shared_ptr<Entity>& vehicle)
{
    if (!vehicle)
    {
        return;
    }

    boost::shared_ptr<ClientboundSetPassengersPacket> packet(new ClientboundSetPassengersPacket(vehicle));

    boost::shared_ptr<TrackedEntity> tracked = static_cast<ServerLevel*>(mLevel)->getTracker()->getTracker(vehicle);

    tracked->broadcast(packet);
}

void Entity::stopRidingBeforeTeleport()
{
    boost::shared_ptr<Entity> vehicle = getVehicle();

    stopRiding();

    fjImmediateBroadcastPassengerPacket(vehicle);
}

void Entity::addPassenger(boost::shared_ptr<Entity> passenger)
{
    if (!mLevel->mIsClientSide && eTYPE_DERIVED_FROM(EntityType::PLAYER, passenger->GetType())
        && (!getControllingPassenger()
            || !eTYPE_DERIVED_FROM(EntityType::PLAYER, getControllingPassenger()->GetType())))
    {
        mPassengers.insert(mPassengers.begin(), passenger);
    }
    else
    {
        mPassengers.push_back(passenger);
    }
}

struct weakPtrEqualsShared
{
    weakPtrEqualsShared(boost::shared_ptr<Entity> shared) : mShared(shared) {}

    bool operator()(boost::weak_ptr<Entity> weak) { return weak.lock() == mShared; }

    boost::shared_ptr<Entity> mShared;
};

void Entity::removePassenger(boost::shared_ptr<Entity> passenger)
{
    if (passenger)
    {
        passenger->getVehicle();
    }

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin(); it != mPassengers.end(); ++it)
    {
        if (!it->lock())
        {
            mPassengers.erase(it);
            break;
        }
    }

    if (passenger)
    {
        std::vector<boost::weak_ptr<Entity> >::iterator it = std::find_if(mPassengers.begin(), mPassengers.end(),
                                                                          weakPtrEqualsShared(passenger));
        if (it != mPassengers.end())
        {
            mPassengers.erase(it);
        }

        passenger->mBoardingCooldown = 60;
    }
}

bool Entity::canAddPassenger(boost::shared_ptr<Entity> passenger)
{
    return getPassengers()->empty();
}

void Entity::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    setPos(x, y, z);
    setRot(yRot, xRot);
}

float Entity::getPickRadius()
{
    return 0.0f;
}

Vec3* Entity::getLookAngle()
{
    return calculateViewVector(mXRot, mYRot);
}

phys::Vec2* Entity::getRotationVector()
{
    return phys::Vec2::newTemp(mXRot, mYRot);
}

Vec3* Entity::getForward()
{
    return Vec3::directionFromRotation(getRotationVector());
}

void Entity::handleInsidePortal(const BlockPos& pos)
{
    if (mPortalCooldown > 0)
    {
        mPortalCooldown = getDimensionChangingDelay();
        return;
    }

    if (!mLevel->mIsClientSide && !(pos == mPortalEntrancePos))
    {
        mPortalEntrancePos = pos;

        BlockPattern::BlockPatternMatch* portal =
            static_cast<PortalBlock*>(Blocks::portal)->getPortalShape(mLevel, mPortalEntrancePos);

        double axis = portal->getForwards()->getAxis() == Direction::Axis::X ? (double)portal->getFrontTopLeft().z
                                                                             : (double)portal->getFrontTopLeft().x;
        double position = portal->getForwards()->getAxis() == Direction::Axis::X ? mZ : mX;
        double edge = portal->getForwards()->getClockWise()->getAxisDirection() == Direction::AxisDirection::NEGATIVE
                          ? 1.0
                          : 0.0;
        double across = Mth::pct(position - edge, axis, axis - (double)portal->getWidth());

        double up = Mth::pct(mY - 1.0, (double)portal->getFrontTopLeft().y,
                             (double)(portal->getFrontTopLeft().y - portal->getHeight()));

        delete mPortalEntranceOffset;
        mPortalEntranceOffset = Vec3::newPermanent(std::abs(across), up, 0.0);
        mPortalEntranceForwards = portal->getForwards();

        delete portal;
    }

    mIsInsidePortal = true;
}

int Entity::getDimensionChangingDelay()
{
    return 300;
}

void Entity::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
}

void Entity::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::EAT_ITEM_PARTICLES)
    {
        spawnEatParticles(ItemInstance(Item::byId(data), 1, 0), 5);
    }
}

void Entity::animateHurt()
{
}

NonNullList<not_null_ptr<ItemInstance> > Entity::getHandSlots()
{
    return NonNullList<not_null_ptr<ItemInstance> >();
}

NonNullList<not_null_ptr<ItemInstance> > Entity::getArmorSlots()
{
    return NonNullList<not_null_ptr<ItemInstance> >();
}

NonNullList<not_null_ptr<ItemInstance> > Entity::getAllSlots()
{
    NonNullList<not_null_ptr<ItemInstance> > hands = getHandSlots();
    NonNullList<not_null_ptr<ItemInstance> > armor = getArmorSlots();

    if (mAllSlots.size() == 0)
    {
        delete[] mAllSlots.items.data;
        mAllSlots = NonNullList<not_null_ptr<ItemInstance> >::withSize(hands.size() + armor.size(),
                                                                       ItemInstance::EMPTY);
    }

    unsigned int index = 0;
    for (unsigned int i = 0; i < hands.size(); ++i)
        mAllSlots[index++] = hands[i];
    for (unsigned int i = 0; i < armor.size(); ++i)
        mAllSlots[index++] = armor[i];

    return mAllSlots;
}

void Entity::setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> itemInstance)
{
}

bool Entity::isOnFire()
{
    bool clientSide = mLevel != NULL && mLevel->mIsClientSide;

    if (mFireImmune)
    {
        return false;
    }

    return mRemainingFireTicks > 0 || (clientSide && getSharedFlag(FLAG_ONFIRE));
}

bool Entity::isPassenger()
{
    return mVehicle.get() != NULL;
}

bool Entity::isVehicle()
{
    return !getPassengers()->empty();
}

bool Entity::isSneaking()
{
    return getSharedFlag(FLAG_SNEAKING);
}

void Entity::setSneaking(bool sneaking)
{
    setSharedFlag(FLAG_SNEAKING, sneaking);
}

bool Entity::isIdle()
{
    return getSharedFlag(FLAG_IDLE);
}

void Entity::setIsIdle(bool idle)
{
    setSharedFlag(FLAG_IDLE, idle);
}

bool Entity::isSprinting()
{
    return getSharedFlag(FLAG_SPRINTING);
}

void Entity::setSprinting(bool sprinting)
{
    setSharedFlag(FLAG_SPRINTING, sprinting);
}

bool Entity::isSwimming()
{
    return getSharedFlag(FLAG_SWIMMING);
}

bool Entity::isSwimmer()
{
    return getSharedFlag(FLAG_SWIMMER);
}

void Entity::startSwimming()
{
    setSharedFlag(FLAG_SWIMMING, true);
}

void Entity::stopSwimming()
{
    setSharedFlag(FLAG_SWIMMING, false);
}

void Entity::setSwimmer(bool swimmer)
{
    setSharedFlag(FLAG_SWIMMER, swimmer);
}

float Entity::getSwimAmount(float partialTicks) const
{
    return Mth::Lerp(mSwimAmountO, mSwimAmount, partialTicks);
}

void Entity::setDamageNearbyMobs(bool damageNearbyMobs)
{
    mAutoSpinAttackTicks = 0;

    setSharedFlag(FLAG_DAMAGE_NEARBY_MOBS, damageNearbyMobs);
}

bool Entity::getDamageNearbyMobs()
{
    return getSharedFlag(FLAG_DAMAGE_NEARBY_MOBS);
}

bool Entity::isGlowing()
{
    return mGlowing || (mLevel->mIsClientSide && getSharedFlag(FLAG_GLOWING));
}

void Entity::setGlowing(bool glowing)
{
    mGlowing = glowing;

    if (mLevel->mIsClientSide)
    {
        return;
    }

    setSharedFlag(FLAG_GLOWING, glowing);
}

bool Entity::isInvisible()
{
    return getSharedFlag(FLAG_INVISIBLE);
}

bool Entity::isInvisibleTo(boost::shared_ptr<Player> player)
{
    if (player->isSpectator())
    {
        return false;
    }

    return isInvisible();
}

Team* Entity::getTeam()
{
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());

        return mLevel->GetScoreboard()->getPlayersTeam(player->getXuid());
    }

    return NULL;
}

bool Entity::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    return isAlliedTo(entity->getTeam());
}

bool Entity::isAlliedTo(Team* team)
{
    if (getTeam() != NULL)
    {
        return getTeam()->isAlliedTo(team);
    }

    return false;
}

void Entity::setInvisible(bool invisible)
{
    setSharedFlag(FLAG_INVISIBLE, invisible);
}

bool Entity::ShouldRenderShadow()
{
    return true;
}

bool Entity::canFly()
{
    return false;
}

Vec3* Entity::getPos(float partialTicks)
{
    if (partialTicks >= 1.0f)
    {
        return Vec3::newTemp(mX, mY, mZ);
    }

    double x = mXo + (mX - mXo) * partialTicks;
    double y = mYo + (mY - mYo) * partialTicks;
    double z = mZo + (mZ - mZo) * partialTicks;

    return Vec3::newTemp(x, y, z);
}

Vec3* Entity::getAttachPos(EEntityLocation location, float partialTicks)
{
    switch (location)
    {
    case ENTITY_LOCATION_FEET:
    case ENTITY_LOCATION_EXPLOSION_POINT:
        return getPos(partialTicks);

    case ENTITY_LOCATION_BODY:
    case ENTITY_LOCATION_WEAPON_ATTACH_POINT:
        return mBoundingBox->getCenter();

    case ENTITY_LOCATION_HEAD:
    {
        Vec3* offset = Vec3::newTemp(0.0, getEyeHeight(), 0.0);
        offset->yRot(-getYHeadRot() * Mth::DEG_TO_RAD);
        return getPos(partialTicks)->add(offset);
    }

    case ENTITY_LOCATION_EYES:
    {
        Vec3* offset = Vec3::newTemp(0.0, getEyeHeight(), 0.0);
        offset->yRot(-getYHeadRot() * Mth::DEG_TO_RAD);
        return getPos(partialTicks)->add(offset);
    }

    case ENTITY_LOCATION_DROP_ATTACH_POINT:
    case ENTITY_LOCATION_BREATHING_POINT:
    case ENTITY_LOCATION_MOUTH:
    {
        Vec3* offset = Vec3::newTemp(0.0, getEyeHeight(), 0.0);
        offset->yRot(-getYHeadRot() * Mth::DEG_TO_RAD);
        return getPos(partialTicks)->add(offset);
    }

    default:
        return getPos(partialTicks);
    }
}

bool Entity::isWeakened()
{
    return getSharedFlag(FLAG_WEAKENED);
}

void Entity::setWeakened(bool weakened)
{
    setSharedFlag(FLAG_WEAKENED, weakened);
}

void Entity::thunderHit(const LightningBolt* lightningBolt)
{
    hurt(DamageSource::LIGHTNING_BOLT, 5.0f);

    mRemainingFireTicks++;

    if (mRemainingFireTicks == 0)
    {
        setSecondsOnFire(8);
    }
}

void Entity::onAboveBubbleCol(bool down)
{
    if (down)
    {
        mYd = mYd - 0.03 < -0.9 ? -0.9 : mYd - 0.03;
    }
    else
    {
        mYd = mYd + 0.1 > 1.8 ? 1.8 : mYd + 0.1;
    }
}

void Entity::onInsideBubbleColumn(bool down)
{
    if (down)
    {
        mYd = mYd - 0.03 < -0.3 ? -0.3 : mYd - 0.03;
    }
    else
    {
        mYd = mYd + 0.3 > 0.4 ? 0.4 : mYd + 0.3;
    }

    mFallDistance = 0.0f;
}

void Entity::killed(boost::shared_ptr<LivingEntity> killed)
{
}

bool Entity::checkInBlock(double x, double y, double z)
{
    BlockPos pos(x, y, z);
    const double fx = x - pos.x;
    const double fy = y - pos.y;
    const double fz = z - pos.z;
    if (!mLevel->hasBlockCubes(mBoundingBox))
        return false;

    const Direction* pushDirection = Direction::UP;
    double closest = DBL_MAX;
    if (!mLevel->isFullAABBBlock(pos.west()) && fx < closest)
    {
        closest = fx;
        pushDirection = Direction::WEST;
    }
    if (!mLevel->isFullAABBBlock(pos.east()) && 1.0 - fx < closest)
    {
        closest = 1.0 - fx;
        pushDirection = Direction::EAST;
    }
    if (!mLevel->isFullAABBBlock(pos.north()) && fz < closest)
    {
        closest = fz;
        pushDirection = Direction::NORTH;
    }
    if (!mLevel->isFullAABBBlock(pos.south()) && 1.0 - fz < closest)
    {
        closest = 1.0 - fz;
        pushDirection = Direction::SOUTH;
    }
    if (!mLevel->isFullAABBBlock(pos.above()) && 1.0 - fy < closest)
        pushDirection = Direction::UP;

    const float speed = mRandom->nextFloat() * 0.2f + 0.1f;
    const float step = static_cast<float>(pushDirection->getAxisDirection()->getStep());
    if (pushDirection->getAxis() == Direction::Axis::X)
    {
        mXd = step * speed;
        mYd *= 0.75;
        mZd *= 0.75;
    }
    const bool airAbove = mLevel->getBlock(pos.above()) == Blocks::air;
    if (pushDirection->getAxis() == Direction::Axis::Y || airAbove)
    {
        mXd *= 0.75;
        mYd = step * speed;
        mZd *= 0.75;
    }
    if (pushDirection->getAxis() == Direction::Axis::Z)
    {
        mXd *= 0.75;
        mYd *= 0.75;
        mZd = step * speed;
    }
    return true;
}

void Entity::spawnEatParticles(const ItemInstance& itemInstance, int count)
{
}

void Entity::makeStuckInWeb()
{
    mFallDistance = 0.0f;
    mFallTicks = 0;
    mIsInWeb = true;
    mFallCount++;
}

std::wstring Entity::getName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }

    return std::wstring(L"");
}

bool Entity::isInvulnerableTo(const DamageSource* source)
{
    return getSharedFlag(FLAG_INVULNERABLE) && source != DamageSource::OUT_OF_WORLD
           && source != DamageSource::CREATIVE_PLAYER && !source->isCreativePlayer();
}

bool Entity::isInvulnerable()
{
    return getSharedFlag(FLAG_INVULNERABLE);
}

void Entity::setInvulnerable(bool invulnerable)
{
    setSharedFlag(FLAG_INVULNERABLE, invulnerable);
}

std::vector<boost::shared_ptr<Entity> >* Entity::getSubEntities()
{
    return NULL;
}

bool Entity::is(boost::shared_ptr<Entity> entity)
{
    return entity == shared_from_this();
}

float Entity::getYHeadRot()
{
    return 0.0f;
}

void Entity::setYHeadRot(float yHeadRot)
{
}

void Entity::setYBodyRot(float yBodyRot)
{
}

bool Entity::isAttackable()
{
    return true;
}

bool Entity::skipAttackInteraction(boost::shared_ptr<Entity> entity)
{
    return false;
}

void Entity::copyPosition(boost::shared_ptr<Entity> entity)
{
    moveTo(entity->mX, entity->mY, entity->mZ, entity->mYRot, entity->mXRot);
}

void Entity::restoreFrom(boost::shared_ptr<Entity> entity)
{
    CompoundTag* tag = new CompoundTag();

    entity->saveWithoutId(tag);
    tag->remove(std::wstring(L"Dimension"));

    load(tag);

    delete tag;

    mPortalCooldown = entity->mPortalCooldown;
    mPortalEntrancePos = entity->mPortalEntrancePos;
    mPortalEntranceOffset = entity->mPortalEntranceOffset;
    mPortalEntranceForwards = entity->mPortalEntranceForwards;
}

boost::shared_ptr<Entity> Entity::changeDimension(int dimension)
{
    if (mLevel->mIsClientSide || mRemoved)
    {
        return boost::shared_ptr<Entity>();
    }

    if (eTYPE_DERIVED_FROM(EntityType::MOB, GetType()) && isLeashed())
    {
        dropLeash(true, true);
    }

    MinecraftServer* server = gMinecraftServer;
    int fromDimension = mDimension;
    ServerLevel* fromLevel = server->getLevel(fromDimension);
    ServerLevel* toLevel = server->getLevel(dimension);

    if (fromDimension == 1 && dimension == 1)
    {
        toLevel = server->getLevel(0);
    }

    bool isFallingBlock = eTYPE_DERIVED_FROM(EntityType::FALLING_BLOCK_ENTITY, GetType());

    if (isFallingBlock)
    {
        mRemoved = true;
        return boost::shared_ptr<Entity>();
    }

    if (toLevel->atEntityLimit(shared_from_this()))
    {
        return boost::shared_ptr<Entity>();
    }

    if (!eTYPE_DERIVED_FROM(EntityType::PLAYER, GetType())
        && !toLevel->canCreateMore(GetType(), Level::eSpawnType_Portal))
    {
        return boost::shared_ptr<Entity>();
    }

    mDimension = toLevel->mDimension->getType()->getId();

    mLevel->removeEntity(shared_from_this());

    mRemoved = false;

    BlockPos spawnPos;

    if (dimension == 1)
    {
        BlockPos* dimensionSpawn = toLevel->getDimensionSpecificSpawn();

        spawnPos = dimensionSpawn != NULL ? *dimensionSpawn : toLevel->getSharedSpawnPos();
    }
    else
    {
        double x = mX;
        double z = mZ;
        int blockX = (int)x;
        int blockZ = (int)z;

        double scale = (double)toLevel->getLevelData()->getHellScale();

        if (dimension == -1)
        {
            x = Mth::clamp(x / scale, toLevel->getWorldBorder()->getMinX() + 16.0,
                           toLevel->getWorldBorder()->getMaxX() - 16.0);
            z = Mth::clamp(z / scale, toLevel->getWorldBorder()->getMinZ() + 16.0,
                           toLevel->getWorldBorder()->getMaxZ() - 16.0);
        }
        else if (dimension == 0)
        {
            x = Mth::clamp(x * scale, toLevel->getWorldBorder()->getMinX() + 16.0,
                           toLevel->getWorldBorder()->getMaxX() - 16.0);
            z = Mth::clamp(z * scale, toLevel->getWorldBorder()->getMinZ() + 16.0,
                           toLevel->getWorldBorder()->getMaxZ() - 16.0);
        }

        mLevel->clampWithinLevelBounds(x, z, 0.0);

        float yRot = mYRot;

        moveTo(x, mY, z, 90.0f, 0.0f);

        PortalForcer* portalForcer = toLevel->getPortalForcer();

        portalForcer->findPortal(shared_from_this(), yRot, blockX, blockZ);

        spawnPos = BlockPos(this);
    }

    fromLevel->tick(shared_from_this(), false);

    boost::shared_ptr<Entity> copy = EntityIO::newEntity(GetType(), toLevel);

    if (copy)
    {
        copy->restoreFrom(shared_from_this());

        if (fromDimension == 1 && dimension == 1)
        {
            BlockPos topPos = toLevel->getTopSolidBlockPos(toLevel->getSharedSpawnPos());

            copy->moveTo(topPos, copy->mYRot, copy->mXRot);
        }
        else
        {
            copy->moveTo(spawnPos, copy->mYRot, copy->mXRot);
        }

        bool forced = copy->mForcedLoading;

        copy->mForcedLoading = true;
        toLevel->addEntity(copy);
        copy->mForcedLoading = forced;

        toLevel->tick(copy, false);
    }

    mRemoved = true;

    fromLevel->resetEmptyTime();
    toLevel->resetEmptyTime();

    return copy;
}

bool Entity::canChangeDimensions()
{
    return true;
}

float Entity::getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos)
{
    return level->getBlock(pos)->getExplosionResistance(shared_from_this());
}

bool Entity::shouldBlockExplode(Explosion* explosion, Level* level, const BlockPos& pos, float power)
{
    return true;
}

int Entity::getMaxFallDistance()
{
    return 3;
}

BlockPos Entity::getPortalEntranceBlock()
{
    return mPortalEntrancePos;
}

Vec3* Entity::getPortalEntranceOffset()
{
    return mPortalEntranceOffset;
}

const Direction* Entity::getPortalEntranceForwards()
{
    return mPortalEntranceForwards;
}

bool Entity::isIgnoringBlockTriggers()
{
    return false;
}

bool Entity::displayFireAnimation()
{
    return isOnFire();
}

void Entity::setUUID(const std::wstring& uuid)
{
    if (uuid.compare(0, uuid.size(), L"") == 0)
    {
        return;
    }

    mUUID = uuid;
    mStringUUID = uuid;
}

std::wstring Entity::getUUID()
{
    return mUUID;
}

std::wstring Entity::getStringUUID()
{
    return mStringUUID;
}

bool Entity::isPushedByWater()
{
    return true;
}

double Entity::getViewScale()
{
    return sViewScale;
}

void Entity::setViewScale(double viewScale)
{
    sViewScale = viewScale;
}

std::wstring Entity::getNetworkName()
{
    return getName();
}

void Entity::setCustomName(const std::wstring& customName, const std::wstring& ownerUUID, bool namedByRestrictedPlayer)
{
    bool createdOnHost = CGameNetworkManager::sInstance.IsHost() && gApp.IsUUIDLocal(ownerUUID) != 0;

    mEntityData->set(DATA_CREATED_ON_HOST, createdOnHost, true);
    mEntityData->set(DATA_NAMED_BY_RESTRICTED_PLAYER, namedByRestrictedPlayer, true);
    mEntityData->set(DATA_CUSTOM_NAME, customName, true);
    mEntityData->set(DATA_OWNER_UUID, ownerUUID, true);
}

std::wstring Entity::getCustomName()
{
    return mEntityData->get(DATA_CUSTOM_NAME);
}

std::wstring Entity::getOwnerUUID()
{
    return mEntityData->get(DATA_OWNER_UUID);
}

bool Entity::wasNamedByRestrictedPlayer()
{
    return mEntityData->get(DATA_NAMED_BY_RESTRICTED_PLAYER);
}

bool Entity::wasNamedOnHost()
{
    return mEntityData->get(DATA_CREATED_ON_HOST);
}

bool Entity::hasCustomName()
{
    bool named = !Entity::getCustomName().empty();

    if (!named)
    {
        return false;
    }

    bool restrictedCase = false;

    if (!gApp.IsUUIDLocal(Entity::getOwnerUUID()))
    {
#ifdef MINECRAFT_PC
        if (gApp.IsConsoleChatCensored())
#else
        if (gApp.IsConsoleChatCensored() || mEntityData->get(DATA_NAMED_BY_RESTRICTED_PLAYER))
#endif
        {
            restrictedCase = true;
        }
    }

    bool hostRestrictedCase = false;

    if (!mEntityData->get(DATA_CREATED_ON_HOST) && restrictedCase)
    {
        hostRestrictedCase = true;
    }

    if (CGameNetworkManager::sInstance.IsHost() && hostRestrictedCase)
    {
        return false;
    }

    if (!CGameNetworkManager::sInstance.IsHost() && restrictedCase)
    {
        return false;
    }

    return true;
}

void Entity::setCustomNameVisible(bool customNameVisible)
{
    mEntityData->set(DATA_CUSTOM_NAME_VISIBLE, customNameVisible, true);
}

bool Entity::isCustomNameVisible()
{
    return mEntityData->get(DATA_CUSTOM_NAME_VISIBLE);
}

void Entity::teleportTo(double x, double y, double z)
{
    mTeleported = true;

    moveTo(x, y, z, mYRot, mXRot);

    mLevel->tick(shared_from_this(), false);
}

bool Entity::shouldShowName()
{
    return isCustomNameVisible();
}

void Entity::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
}

const Direction* Entity::getDirection()
{
    return Direction::from2DDataValue(Mth::floor(mYRot * 4.0 / 360.0 + 0.5) & 3);
}

const Direction* Entity::getMotionDirection()
{
    return getDirection();
}

bool Entity::broadcastToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    return true;
}

AABB* Entity::getBoundingBoxForCulling()
{
    return mBoundingBox;
}

void Entity::setBoundingBox(AABB* boundingBox)
{
    mBoundingBox->set(boundingBox);
}

float Entity::getEyeHeight()
{
    return (double)(mBbHeight * 0.85f);
}

void Entity::setOutsideWorldBorder(bool outsideWorldBorder)
{
    mOutsideWorldBorder = outsideWorldBorder;
}

bool Entity::setSlot(int slot, const not_null_ptr<ItemInstance>& itemInstance)
{
    return false;
}

InteractionResult::EInteractionResult Entity::interactAt(const boost::shared_ptr<Player>& player, Vec3* pos,
                                                         InteractionHand::EInteractionHand hand)
{
    return InteractionResult::eInteractionResult_Pass;
}

bool Entity::ignoreExplosion()
{
    return false;
}

void Entity::doEnchantDamageEffects(boost::shared_ptr<LivingEntity> attacker, boost::shared_ptr<Entity> target)
{
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
    {
        EnchantmentHelper::doPostHurtEffects(boost::dynamic_pointer_cast<LivingEntity>(target),
                                             boost::static_pointer_cast<Entity>(attacker));
    }

    EnchantmentHelper::doPostDamageEffects(attacker, target);
}

void Entity::startSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
}

void Entity::stopSeenByPlayer(boost::shared_ptr<ServerPlayer> player)
{
}

float Entity::rotate(Rotation* rotation)
{
    float yRot = Mth::wrapDegrees(mYRot);

    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise90:
        return yRot + 90.0f;

    case Rotation::eTYPE_Clockwise180:
        return yRot + 180.0f;

    case Rotation::eTYPE_CounterClockwise90:
        return yRot + 270.0f;

    default:
        return yRot;
    }
}

float Entity::mirror(Mirror* mirror)
{
    float yRot = Mth::wrapDegrees(mYRot);

    if (mirror->getType() == Mirror::eTYPE_LeftRight)
    {
        return -yRot;
    }

    if (mirror->getType() == Mirror::eTYPE_FrontBack)
    {
        return 180.0f - yRot;
    }

    return yRot;
}

bool Entity::onlyOpCanSetNbt()
{
    return false;
}

bool Entity::checkAndResetTeleportedFlag()
{
    bool teleported = mTeleported;

    mTeleported = false;

    return teleported;
}

boost::shared_ptr<Entity> Entity::getControllingPassenger()
{
    if (mPassengers.empty())
    {
        return boost::shared_ptr<Entity>();
    }

    return mPassengers.at(0).lock();
}

bool Entity::hasPassenger(boost::shared_ptr<Entity> passenger)
{
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers->begin(); it != passengers->end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (rider && rider == passenger)
        {
            return true;
        }
    }

    return false;
}

bool Entity::hasPassenger(unsigned long long type)
{
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers->begin(); it != passengers->end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (rider)
        {
            unsigned long long riderType = rider->GetType();

            bool matches;

            if ((type & 0x3FULL) == 0)
            {
                matches = (riderType & type) == type;
            }
            else
            {
                matches = riderType == type;
            }

            if (matches)
            {
                return true;
            }
        }
    }

    return false;
}

void Entity::fillIndirectPassengers(unsigned long long type, std::set<boost::shared_ptr<Entity> >* passengers)
{
    for (std::vector<boost::weak_ptr<Entity> >::iterator it = mPassengers.begin(); it != mPassengers.end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (!rider)
        {
            continue;
        }

        unsigned long long riderType = rider->GetType();

        bool matches;

        if ((type & 0x3FULL) == 0)
        {
            matches = (riderType & type) == type;
        }
        else
        {
            matches = riderType == type;
        }

        if (matches)
        {
            passengers->insert(rider);
        }

        rider->fillIndirectPassengers(type, passengers);
    }
}

std::set<boost::shared_ptr<Entity> >* Entity::getIndirectPassengers(unsigned long long type)
{
    std::set<boost::shared_ptr<Entity> >* passengers = new std::set<boost::shared_ptr<Entity> >();

    fillIndirectPassengers(type, passengers);

    return passengers;
}

std::set<boost::shared_ptr<Entity> >* Entity::getIndirectPassengers()
{
    std::set<boost::shared_ptr<Entity> >* passengers = new std::set<boost::shared_ptr<Entity> >();

    fillIndirectPassengers(EntityType::ENTITY, passengers);

    return passengers;
}

boost::shared_ptr<Entity> Entity::getRootVehicle()
{
    boost::shared_ptr<Entity> root = shared_from_this();

    while (root->isPassenger())
    {
        root = root->getVehicle();
    }

    return root;
}

bool Entity::isPassengerOfSameVehicle(boost::shared_ptr<Entity> entity)
{
    return getRootVehicle() == entity->getRootVehicle();
}

bool Entity::hasIndirectPassenger(boost::shared_ptr<Entity> passenger)
{
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers->begin(); it != passengers->end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (rider == passenger)
        {
            return true;
        }

        if (rider && rider->hasIndirectPassenger(passenger))
        {
            return true;
        }
    }

    return false;
}

bool Entity::hasIndirectPassenger(unsigned long long type)
{
    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();

    for (std::vector<boost::weak_ptr<Entity> >::iterator it = passengers->begin(); it != passengers->end(); ++it)
    {
        boost::shared_ptr<Entity> rider = it->lock();

        if (rider)
        {
            unsigned long long riderType = rider->GetType();

            bool matches;

            if ((type & 0x3FULL) == 0)
            {
                matches = (riderType & type) == type;
            }
            else
            {
                matches = riderType == type;
            }

            if (matches)
            {
                return true;
            }
        }
    }

    return false;
}

bool Entity::isControlledByLocalInstance()
{
    boost::shared_ptr<Entity> controller = getControllingPassenger();

    if (instanceof(controller, EntityType::PLAYER))
    {
        return boost::static_pointer_cast<Player>(controller)->isLocalPlayer();
    }

    return !mLevel->mIsClientSide;
}

boost::shared_ptr<Entity> Entity::getVehicle()
{
    return mVehicle;
}

int Entity::getPistonPushReaction()
{
    return 0;
}

SoundSource::ESoundSource Entity::getSoundSource()
{
    return SoundSource::eSoundSource_Neutral;
}

bool Entity::isCreative()
{
    return false;
}

bool Entity::isDespawnProtected()
{
    return false;
}

void Entity::setDespawnProtected()
{
}

bool Entity::couldWander()
{
    return false;
}

bool Entity::canCreateParticles()
{
    return true;
}

void Entity::stopCurrentLerp()
{
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    mXo = mX;
    mYo = mY;
    mZo = mZ;
    mXRotO = mXRot;
    mYRotO = mYRot;
}

bool Entity::controllingPassengerIsA(unsigned long long type)
{
    boost::shared_ptr<Entity> controller = getControllingPassenger();

    if (!controller)
    {
        return false;
    }

    unsigned long long controllerType = controller->GetType();

    if ((type & 0x3FULL) == 0)
    {
        return (controllerType & type) == type;
    }

    return controllerType == type;
}

void Entity::DoAutoAttackOnTouch(boost::shared_ptr<Entity> entity)
{
}

bool Entity::isImmersedInWater()
{
    Vec3* pos = getPos(0.0f);

    double size = mBoundingBox->getSize();

    Vec3* bottomPos = Vec3::newTemp(pos->x, pos->y - size, pos->z);
    Vec3* topPos = Vec3::newTemp(pos->x, pos->y + size, pos->z);

    Material* topMaterial = mLevel->getBlock(BlockPos(topPos))->defaultBlockState()->getMaterial();
    Material* bottomMaterial = mLevel->getBlock(BlockPos(bottomPos))->defaultBlockState()->getMaterial();

    return topMaterial != Material::AIR && bottomMaterial != Material::AIR;
}

void Entity::tickLeash()
{
    if (mLeashInfoTag != NULL)
    {
        restoreLeashFromSave();
    }

    if (!mLeashed)
    {
        return;
    }

    if (!isAlive())
    {
        dropLeash(true, true);
        return;
    }

    boost::shared_ptr<Entity> holder = getLeashHolder();

    if (!holder || holder->mRemoved)
    {
        dropLeash(true, true);
    }
}

void Entity::dropLeash(bool broadcast, bool dropItem)
{
    if (!mLeashed)
    {
        return;
    }

    mLeashed = false;
    mLeashHolder.reset();

    if (!mLevel->mIsClientSide && dropItem)
    {
        spawnAtLocation(Items::lead, 1);
    }

    if (!mLevel->mIsClientSide && broadcast && dynamic_cast<ServerLevel*>(mLevel) != NULL)
    {
        dynamic_cast<ServerLevel*>(mLevel)->getTracker()->broadcast(
            shared_from_this(), boost::shared_ptr<ClientboundSetEntityLinkPacket>(new ClientboundSetEntityLinkPacket(
                                    shared_from_this(), boost::shared_ptr<Entity>())));
    }
}

bool Entity::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return false;
}

bool Entity::isLeashed()
{
    return mLeashed;
}

boost::shared_ptr<Entity> Entity::getLeashHolder()
{
    return mLeashHolder.lock();
}

bool Entity::isLeashedToPlayer(const boost::shared_ptr<Player>& player)
{
    return isLeashed() && getLeashHolder() == player;
}

void Entity::setLeashedTo(boost::shared_ptr<Entity> leashHolder, bool broadcast)
{
    mLeashed = true;
    mLeashHolder = boost::weak_ptr<Entity>(leashHolder);

    ServerLevel* serverLevel = dynamic_cast<ServerLevel*>(mLevel);

    if (serverLevel != NULL && !mLevel->mIsClientSide && broadcast)
    {
        serverLevel->getTracker()->broadcast(
            shared_from_this(), boost::shared_ptr<ClientboundSetEntityLinkPacket>(
                                    new ClientboundSetEntityLinkPacket(2, shared_from_this(), leashHolder)));
    }

    if (isPassenger())
    {
        stopRiding();
    }
}

void Entity::restoreLeashFromSave()
{
    if (mLeashed && mLeashInfoTag != NULL)
    {
        if (mLeashInfoTag->contains(std::wstring(L"UUID")))
        {
            std::wstring uuid = mLeashInfoTag->getString(std::wstring(L"UUID"));

            std::vector<boost::shared_ptr<Entity> >* nearby = mLevel->getEntitiesOfClass(
                typeid(LivingEntity), mBoundingBox->grow(10.0, 10.0, 10.0));

            for (std::vector<boost::shared_ptr<Entity> >::iterator it = nearby->begin(); it != nearby->end(); ++it)
            {
                boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(*it);

                if (living->getUUID() == uuid)
                {
                    setLeashedTo(living, true);
                    break;
                }
            }

            delete nearby;
        }
        else if (mLeashInfoTag->contains(std::wstring(L"X"), 99) && mLeashInfoTag->contains(std::wstring(L"Y"), 99)
                 && mLeashInfoTag->contains(std::wstring(L"Z"), 99))
        {
            BlockPos pos(mLeashInfoTag->getInt(std::wstring(L"X")), mLeashInfoTag->getInt(std::wstring(L"Y")),
                         mLeashInfoTag->getInt(std::wstring(L"Z")));

            boost::shared_ptr<LeashFenceKnotEntity> knot = LeashFenceKnotEntity::findKnotAt(mLevel, pos);

            if (!knot)
            {
                knot = LeashFenceKnotEntity::createAndAddKnot(mLevel, pos);
            }

            setLeashedTo(knot, true);
        }
        else
        {
            dropLeash(false, true);
        }
    }

    boost::shared_ptr<Entity> holder = mLeashHolder.lock();

    if (holder)
    {
        setLeashedTo(holder, true);
    }

    mLeashInfoTag = NULL;
}

CompoundTag* Entity::getAdditionContructionTag()
{
    return NULL;
}
