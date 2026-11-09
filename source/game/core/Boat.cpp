#include "game/core/Boat.hpp"
#include "game/core/StringIds.hpp"
#include "game/item/ItemId.hpp"
#include "game/core/EntityType.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <vector>

#include "game/core/Abilities.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/PushableBySelector.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/entity/mob/IndirectEntityDamageSource.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundPaddleBoatPacket.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

const EntityDataAccessor<int>* Boat::DATA_HURT_TIME = 0;
const EntityDataAccessor<int>* Boat::DATA_HURT_DIR = 0;
const EntityDataAccessor<float>* Boat::DATA_DAMAGE = 0;
const EntityDataAccessor<int>* Boat::DATA_TYPE = 0;
const EntityDataAccessor<bool>* Boat::DATA_ID_PADDLE[2] = {0, 0};
const EntityDataAccessor<int>* Boat::DATA_BUBBLE_TIME = 0;
const EntityDataAccessor<bool>* Boat::DATA_SINKING = 0;

namespace
{
typedef std::vector<boost::weak_ptr<Entity> > PassengerList;
}

Boat::Boat(Level* level) : Entity(level, true)
{
    _init();
}

Boat::Boat(Level* level, double x, double y, double z) : Entity(level, true)
{
    _init();

    const AABB* bounds = level->mDimension->getPlayerConstraints(BlockPos(x, y, z))->shrink(1.0);
    const double clampedX = Mth::clamp(x, bounds->mMinX, bounds->mMaxX);
    const double clampedZ = Mth::clamp(z, bounds->mMinZ, bounds->mMaxZ);
    setPos(clampedX, y, clampedZ);
    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mXo = clampedX;
    mYo = y;
    mZo = clampedZ;
}

void Boat::staticCtor()
{
    DATA_HURT_TIME = SynchedEntityData::defineId<int>(EntityType::BOAT, EntityType::ENTITY, EntityDataSerializers::INT);
    DATA_HURT_DIR = SynchedEntityData::defineId<int>(EntityType::BOAT, EntityType::ENTITY, EntityDataSerializers::INT);
    DATA_DAMAGE = SynchedEntityData::defineId<float>(EntityType::BOAT, EntityType::ENTITY,
                                                     EntityDataSerializers::FLOAT);
    DATA_TYPE = SynchedEntityData::defineId<int>(EntityType::BOAT, EntityType::ENTITY, EntityDataSerializers::INT);
    DATA_ID_PADDLE[0] = SynchedEntityData::defineId<bool>(EntityType::BOAT, EntityType::ENTITY,
                                                          EntityDataSerializers::BOOLEAN);
    DATA_ID_PADDLE[1] = SynchedEntityData::defineId<bool>(EntityType::BOAT, EntityType::ENTITY,
                                                          EntityDataSerializers::BOOLEAN);
    DATA_BUBBLE_TIME = SynchedEntityData::defineId<int>(EntityType::BOAT, EntityType::ENTITY,
                                                        EntityDataSerializers::INT);
    DATA_SINKING = SynchedEntityData::defineId<bool>(EntityType::BOAT, EntityType::ENTITY,
                                                     EntityDataSerializers::BOOLEAN);
}

void Boat::_init()
{
    fjDerivedCtorCalls();

    mPaddlePositions[0] = 0.0f;
    mPaddlePositions[1] = 0.0f;
    mInvFriction = 0.0f;
    mOutOfControlTicks = 0;
    mDeltaRotation = 0.0f;
    mLerpSteps = 0;
    mLerpX = 0.0;
    mLerpY = 0.0;
    mLerpZ = 0.0;
    mLerpYRot = 0.0;
    mLerpXRot = 0.0;
    mInputLeft = false;
    mInputRight = false;
    mInputUp = false;
    mInputDown = false;
    mWaterLevel = 0.0;
    mLandFriction = 0.0f;
    mStatus = IN_WATER;
    mOldStatus = IN_WATER;
    mLastYd = 0.0;
    mIsAboveBubbleColumn = false;
    mBubbleColumnDirectionIsDown = false;
    mBubbleMultiplier = 0.0f;
    mBubbleAngle = 0.0f;
    mBubbleAngleO = 0.0f;
    mRockPhase = 0.0;
    mBlocksBuilding = true;
    setSize(1.375f, 0.6625f);
    mCategories |= EntityCategory::eEntityCategory_Boat | EntityCategory::eEntityCategory_Rideable;
}

bool Boat::makeStepSound()
{
    return false;
}

void Boat::defineSynchedData()
{
    mEntityData->createDateItem(DATA_HURT_TIME, 0);
    mEntityData->createDateItem(DATA_HURT_DIR, 1);
    mEntityData->createDateItem(DATA_DAMAGE, 0.0f);
    mEntityData->createDateItem(DATA_TYPE, static_cast<int>(OAK));
    mEntityData->createDateItem(DATA_ID_PADDLE[0], false);
    mEntityData->createDateItem(DATA_ID_PADDLE[1], false);
    mEntityData->createDateItem(DATA_BUBBLE_TIME, 0);
    mEntityData->createDateItem(DATA_SINKING, false);
}

AABB* Boat::getCollideAgainstBox(boost::shared_ptr<Entity> entity)
{
    if (entity->isPushable(shared_from_this()))
        return entity->mBoundingBox;
    return NULL;
}

AABB* Boat::getCollideBox()
{
    return mBoundingBox;
}

bool Boat::isPushable(boost::shared_ptr<Entity> entity)
{
    return true;
}

double Boat::getRideHeight()
{
    return -0.1f;
}

Boat::Type Boat::getType()
{
    return static_cast<Type>(mEntityData->get(DATA_TYPE));
}

Item* Boat::getDropItem()
{
    switch (getType())
    {
    case OAK:
        return Items::boat;
    default:
        return Items::boat;
    case SPRUCE:
        return Items::spruceBoat;
    case BIRCH:
        return Items::birchBoat;
    case JUNGLE:
        return Items::jungleBoat;
    case ACACIA:
        return Items::acaciaBoat;
    case DARK_OAK:
        return Items::darkOakBoat;
    }
}

bool Boat::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    if (source != NULL && dynamic_cast<const IndirectEntityDamageSource*>(source) != NULL && source->getEntity() != NULL
        && hasPassenger(source->getEntity()))
    {
        return false;
    }

    if (mLevel->mIsClientSide || mRemoved)
        return true;

    if (source != NULL && dynamic_cast<const EntityDamageSource*>(source) != NULL)
    {
        boost::shared_ptr<Entity> attacker = source->getDirectEntity();
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType())
            && !boost::static_pointer_cast<Player>(attacker)->isAllowedToHurtEntity(shared_from_this()))
        {
            return false;
        }
    }

    setHurtDir(-getHurtDir());
    setHurtTime(10);
    if (isVehicle() && hasPassenger(source->getEntity()))
        damage += 1.0f;
    setDamage(getDamage() + damage * 10.0f);
    markHurt();

    const bool creativePlayer = source->getEntity() != NULL
                                && eTYPE_DERIVED_FROM(EntityType::PLAYER, source->getEntity()->GetType())
                                && boost::static_pointer_cast<Player>(source->getEntity())->mAbilities.mInstabuild;

    if (creativePlayer || getDamage() > 40.0f)
    {
        if (!creativePlayer && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
            spawnAtLocation(getDropItem(), 1, 0.0f);

        remove();
    }

    return true;
}

int Boat::getBubbleTime()
{
    return mEntityData->get(DATA_BUBBLE_TIME);
}

void Boat::setBubbleTime(int time)
{
    mEntityData->set(DATA_BUBBLE_TIME, time, true);
}

void Boat::onAboveBubbleCol(bool down)
{
    if (!mLevel->mIsClientSide)
    {
        mIsAboveBubbleColumn = true;
        mBubbleColumnDirectionIsDown = down;
        if (getBubbleTime() == 0)
            setBubbleTime(60);
    }

    if (mLevel->mRandom->nextInt(4) == 0)
    {
        mLevel->addParticle(PARTICLE_SPLASH, mX + static_cast<double>(mRandom->nextFloat()), mY + 0.7,
                            mZ + static_cast<double>(mRandom->nextFloat()), 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
}

void Boat::push(boost::shared_ptr<Entity> entity)
{
    const bool isBoat = eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType());
    if (isBoat)
    {
        if (entity->mBoundingBox->mMinY < mBoundingBox->mMaxY)
            Entity::push(entity);
    }
    else if (entity->mBoundingBox->mMinY <= mBoundingBox->mMinY)
    {
        Entity::push(entity);
    }
}

void Boat::animateHurt()
{
    setHurtDir(-getHurtDir());
    setHurtTime(10);
    setDamage(getDamage() * 11.0f);
}

bool Boat::isPickable()
{
    return !mRemoved;
}

void Boat::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    mLerpX = x;
    mLerpY = y;
    mLerpZ = z;
    mLerpYRot = yRot;
    mLerpXRot = xRot;
    mLerpSteps = 10;
}

const Direction* Boat::getMotionDirection()
{
    return getDirection()->getClockWise();
}

Boat::EStatus Boat::isUnderwater()
{
    const AABB* box = mBoundingBox;
    const double surfaceY = box->mMaxY + 0.001;
    const int minX = Mth::floor(box->mMinX);
    const int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    const int minY = Mth::floor(box->mMaxY);
    const int maxY = Mth::ceil(static_cast<float>(surfaceY));
    const int minZ = Mth::floor(box->mMinZ);
    const int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    bool underWater = false;
    BlockPos pos;

    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                pos = BlockPos(x, y, z);
                const BlockState* state = mLevel->getBlockState(mTickCache, pos);
                if (state->getMaterial() != Material::WATER)
                    state = mLevel->getExtraBlockState(pos);
                if (state->getMaterial() == Material::WATER
                    && surfaceY < LiquidBlock::getLiquidHeight(state, mLevel, pos))
                {
                    if (state->getValue(LiquidBlock::LEVEL) != 0)
                        return UNDER_FLOWING_WATER;
                    underWater = true;
                }
            }
        }
    }

    return underWater ? UNDER_WATER : NOT_UNDERWATER;
}

float Boat::getGroundFriction()
{
    const AABB* box = AABB::newTemp(mBoundingBox->mMinX, mBoundingBox->mMinY - 0.001, mBoundingBox->mMinZ,
                                    mBoundingBox->mMaxX, mBoundingBox->mMinY, mBoundingBox->mMaxZ);
    const int minX = Mth::floor(box->mMinX) - 1;
    const int maxX = Mth::ceil(static_cast<float>(box->mMaxX)) + 1;
    const int minY = Mth::floor(box->mMinY) - 1;
    const int maxY = Mth::ceil(static_cast<float>(box->mMaxY)) + 1;
    const int minZ = Mth::floor(box->mMinZ) - 1;
    const int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ)) + 1;
    std::vector<AABB*> collisions;
    float friction = 0.0f;
    int count = 0;
    BlockPos pos(0, 0, 0);

    for (int x = minX; x < maxX; ++x)
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            const int edges = (x != minX && x != maxX - 1 ? 0 : 1) + (z != minZ && z != maxZ - 1 ? 0 : 1);
            if (edges != 2)
            {
                for (int y = minY; y < maxY; ++y)
                {
                    if (edges <= 0 || (y != minY && y != maxY - 1))
                    {
                        pos = BlockPos(x, y, z);
                        const BlockState* state = mLevel->getBlockState(mTickCache, pos);
                        state->addCollissionAABBs(mLevel, pos, box, &collisions, shared_from_this(), false);
                        if (!collisions.empty())
                        {
                            friction += state->getBlock()->mFriction;
                            ++count;
                        }
                        collisions.clear();
                    }
                }
            }
        }
    }

    if (count == 0)
        return 0.0f;
    return friction / static_cast<float>(count);
}

bool Boat::checkInWater()
{
    const AABB* box = mBoundingBox;
    const int minX = Mth::floor(box->mMinX);
    const int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    const int minY = Mth::floor(box->mMinY);
    const int maxY = Mth::ceil(static_cast<float>(box->mMinY + 0.001));
    const int minZ = Mth::floor(box->mMinZ);
    const int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));
    bool inWater = false;
    mWaterLevel = -DBL_MAX;
    BlockPos pos;

    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                pos = BlockPos(x, y, z);
                const BlockState* state = mLevel->getBlockState(mTickCache, pos);
                if (state->getMaterial() != Material::WATER)
                    state = mLevel->getExtraBlockState(pos);
                if (state->getMaterial() == Material::WATER)
                {
                    const double height = LiquidBlock::getLiquidHeight(state, mLevel, pos);
                    mWaterLevel = std::max(height, mWaterLevel);
                    inWater |= box->mMinY < height;
                }
            }
        }
    }

    return inWater;
}

Boat::EStatus Boat::getStatus()
{
    const EStatus underwaterStatus = isUnderwater();
    if (underwaterStatus != NOT_UNDERWATER)
    {
        mWaterLevel = mBoundingBox->mMaxY;
        return underwaterStatus;
    }

    if (checkInWater())
        return IN_WATER;

    const float friction = getGroundFriction();
    if (friction > 0.0f)
    {
        mLandFriction = friction;
        return ON_LAND;
    }

    return IN_AIR;
}

const SoundEvent* Boat::getPaddleSound()
{
    switch (getStatus())
    {
    case IN_WATER:
    case UNDER_WATER:
    case UNDER_FLOWING_WATER:
        return SoundEvents::BOAT_PADDLE_WATER;
    case ON_LAND:
        return SoundEvents::BOAT_PADDLE_LAND;
    default:
        return NULL;
    }
}

bool Boat::getIsSinking()
{
    return mEntityData->get(DATA_SINKING);
}

void Boat::tickLerp()
{
    if (mLerpSteps > 0 && !isControlledByLocalInstance())
    {
        const double x = mX + (mLerpX - mX) / mLerpSteps;
        const double y = mY + (mLerpY - mY) / mLerpSteps;
        const double z = mZ + (mLerpZ - mZ) / mLerpSteps;
        const float yRotDelta = static_cast<float>(Mth::wrapDegrees(mLerpYRot - mYRot));
        mYRot += yRotDelta / static_cast<float>(mLerpSteps);
        mXRot += static_cast<float>(mLerpXRot - mXRot) / static_cast<float>(mLerpSteps);
        --mLerpSteps;
        setPos(x, y, z);
        setRot(mYRot, mXRot);
    }
}

void Boat::setPaddleState(bool left, bool right)
{
    mEntityData->set(DATA_ID_PADDLE[0], left, true);
    mEntityData->set(DATA_ID_PADDLE[1], right, true);
}

bool Boat::findBlockHeightOnPlane(const AABB* box, float& height, BlockPos& pos)
{
    const int minX = Mth::floor(box->mMinX);
    const int maxX = Mth::ceil(static_cast<float>(box->mMaxX));
    const int minZ = Mth::floor(box->mMinZ);
    const int maxZ = Mth::ceil(static_cast<float>(box->mMaxZ));

    for (int x = minX; x < maxX; ++x)
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            pos = BlockPos(x, pos.y, z);
            const BlockState* state = mLevel->getBlockState(mTickCache, pos);
            if (state->getMaterial() != Material::WATER)
                state = mLevel->getExtraBlockState(pos);
            if (state->getMaterial() == Material::WATER)
                height = std::max(height, LiquidBlock::getLiquidHeight(state, mLevel, pos));

            if (height >= 1.0f)
                return false;
        }
    }

    return true;
}

float Boat::getWaterLevelAbove()
{
    const AABB* box = mBoundingBox;
    const int minY = Mth::floor(box->mMaxY);
    const int maxY = Mth::ceil(static_cast<float>(box->mMaxY - mLastYd));
    BlockPos pos(0, 0, 0);

    for (int y = minY; y < maxY; ++y)
    {
        float height = 0.0f;
        pos = BlockPos(pos.x, y, pos.z);
        if (findBlockHeightOnPlane(box, height, pos) && height < 1.0f)
            return static_cast<float>(pos.y) + height;
    }

    return static_cast<float>(maxY) + 1.0f;
}

void Boat::floatBoat()
{
    double momentum = Vec3::newTemp(mXd, 0.0, mZd)->length() * 30.0 + 1.0;
    if (mLevel->mRandom->nextInt(30) == 0)
        momentum *= 10.0;
    mRockPhase += 0.05 * momentum;

    double buoyancy = 0.0;
    double rock = 0.0;

    const BlockPos pos(mX, mY, mZ);
    const Material* material = mLevel->getBlockState(mTickCache, pos)->getMaterial();
    const Material* extraMaterial = mLevel->getExtraBlockState(pos)->getMaterial();
    const Material* materialAbove = mLevel->getBlockState(mTickCache, pos.above())->getMaterial();
    const Material* extraMaterialAbove = mLevel->getExtraBlockState(pos.above())->getMaterial();
    const double buoyancyOffset = 0.1;
    bool floating = false;

    if ((material == Material::WATER || extraMaterial == Material::WATER) && materialAbove == Material::AIR
        && !getIsSinking())
    {
        buoyancy = (1.0 - (mY - pos.y)) * 0.9 + buoyancyOffset;
        rock = (std::sin(mRockPhase) + 1.0) * 0.035;
        mOutOfControlTicks = 0;
        mInvFriction = 0.9f;
        floating = true;
    }
    else if ((material == Material::WATER || extraMaterial == Material::WATER)
             && (materialAbove == Material::WATER || extraMaterialAbove == Material::WATER))
    {
        buoyancy = 1.0;
        mInvFriction = 0.9f;
        ++mOutOfControlTicks;
    }
    else if (material == Material::BUBBLE_COLUMN && materialAbove == Material::BUBBLE_COLUMN)
    {
        buoyancy = 1.0;
        mInvFriction = 0.9f;
        mOutOfControlTicks = 60;
    }
    else if (material == Material::AIR || material == Material::TOP_SNOW)
    {
        const Material* materialBelow = mLevel->getBlockState(mTickCache, pos.below())->getMaterial();
        if (materialBelow != Material::WATER)
            materialBelow = mLevel->getExtraBlockState(pos.below())->getMaterial();

        mInvFriction = 1.0f;
        if (mOnGround)
            mInvFriction = mLevel->getBlock(pos.below())->mFriction;
        else if (materialBelow == Material::WATER)
            mInvFriction = 0.9f;
    }

    mXd *= mInvFriction;
    mZd *= mInvFriction;
    mDeltaRotation *= mInvFriction;

    if (floating && !getIsSinking())
    {
        setPos(mX, getWaterLevelAbove() - mBbHeight + 0.2 + std::sin(mRockPhase) * 0.035, mZ);
        mYd = 0.0;
        return;
    }

    mYd = (mYd - 0.04) * 0.98;
    if (buoyancy > 0.0)
        mYd = std::min(mYd * 0.7 + 0.05, (buoyancy - rock - buoyancyOffset) * 0.15);
}

void Boat::controlBoat()
{
    if (!isVehicle())
        return;

    float acceleration = 0.0f;
    if (mInputLeft)
        mDeltaRotation += -1.0f;
    if (mInputRight)
        mDeltaRotation += 1.0f;
    if (mInputRight != mInputLeft && !mInputUp && !mInputDown)
        acceleration += 0.005f;

    mYRot += mDeltaRotation;
    if (mInputUp)
        acceleration += 0.04f;
    if (mInputDown)
        acceleration -= 0.005f;

    mXd += Mth::sin(-mYRot * Mth::DEG_TO_RAD) * acceleration;
    mZd += Mth::cos(mYRot * Mth::DEG_TO_RAD) * acceleration;
    setPaddleState((mInputRight && !mInputLeft) || mInputUp, (mInputLeft && !mInputRight) || mInputUp);
}

bool Boat::getPaddleState(int paddle)
{
    return mEntityData->get(DATA_ID_PADDLE[paddle]) && getControllingPassenger() != NULL;
}

void Boat::setIsSinking(bool sinking)
{
    mEntityData->set(DATA_SINKING, sinking, true);
}

void Boat::tickBubbleColumn()
{
    if (mLevel->mIsClientSide)
    {
        if (getBubbleTime() > 0)
            mBubbleMultiplier += 0.05f;
        else
            mBubbleMultiplier -= 0.1f;
        mBubbleMultiplier = Mth::clamp(mBubbleMultiplier, 0.0f, 1.0f);
        mBubbleAngleO = mBubbleAngle;
        mBubbleAngle = 10.0f * Mth::sin(0.5f * static_cast<float>(mLevel->getGameTime())) * mBubbleMultiplier;
    }
    else
    {
        if (!mIsAboveBubbleColumn)
            setBubbleTime(0);

        int bubbleTime = getBubbleTime();
        if (bubbleTime > 0)
        {
            --bubbleTime;
            setBubbleTime(bubbleTime);
            const int elapsed = 60 - bubbleTime - 1;
            if (elapsed > 0 && bubbleTime <= 0)
            {
                setBubbleTime(0);
                if (mBubbleColumnDirectionIsDown)
                {
                    mYd -= 0.7f;
                    mOutOfControlTicks = 60;
                    setIsSinking(true);
                }
            }
            mIsAboveBubbleColumn = false;
        }
    }
}

void Boat::tick()
{
    PIXBeginNamedEvent(0.0f, "Boat tick");

    mOldStatus = mStatus;
    mStatus = getStatus();
    if (mStatus == UNDER_WATER || mStatus == UNDER_FLOWING_WATER)
        ++mOutOfControlTicks;
    else
        mOutOfControlTicks = 0;

    if (!mLevel->mIsClientSide && (mOutOfControlTicks >= 60 || getIsSinking()))
        ejectPassengers();

    if (getHurtTime() > 0)
        setHurtTime(getHurtTime() - 1);
    if (getDamage() > 0.0f)
        setDamage(getDamage() - 1.0f);

    mXo = mX;
    mYo = mY;
    mZo = mZ;

    Entity::tick();
    tickLerp();
    tickLeash();

    if (isControlledByLocalInstance())
    {
        if (getPassengers()->empty()
            || !eTYPE_DERIVED_FROM(EntityType::PLAYER,
                                   static_cast<const PassengerList*>(getPassengers())->at(0).lock()->GetType()))
            setPaddleState(false, false);

        floatBoat();
        if (mLevel->mIsClientSide)
        {
            controlBoat();
            boost::static_pointer_cast<LocalPlayer>(getControllingPassenger())
                ->mPacketListener->send(
                    boost::shared_ptr<Packet>(new ServerboundPaddleBoatPacket(getPaddleState(0), getPaddleState(1))));
        }
        move(eMoverType_Self, mXd, mYd, mZd, false);
    }
    else
    {
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
    }

    tickBubbleColumn();

    const float paddleStroke = 3.1415927f / 8.0f;
    const float paddleSplash = 3.1415927f / 4.0f;
    const float paddleCycle = 3.1415927f * 2.0f;
    for (int i = 0; i < 2; ++i)
    {
        if (getPaddleState(i))
        {
            if (!isSilent() && std::fmod(mPaddlePositions[i], paddleCycle) <= paddleSplash
                && std::fmod(mPaddlePositions[i] + paddleStroke, paddleCycle) >= paddleSplash)
            {
                const SoundEvent* sound = getPaddleSound();
                if (sound != NULL)
                {
                    const Vec3* view = getViewVector(1.0f);
                    const double xOffset = i == 1 ? -view->z : view->z;
                    const double zOffset = i == 1 ? view->x : -view->x;
                    mLevel->playSound(boost::shared_ptr<Player>(), mX + xOffset, mY, mZ + zOffset, sound,
                                      getSoundSource(), 1.0f, 0.8f + 0.4f * mRandom->nextFloat(), 16.0f);
                }
            }
            mPaddlePositions[i] += paddleStroke;
        }
        else
        {
            mPaddlePositions[i] = 0.0f;
        }
    }

    checkInsideBlocks();

    PushableBySelector selector(shared_from_this());
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        shared_from_this(), mBoundingBox->grow(0.2f, -0.01f, 0.2f), &selector, NULL);
    if (!entities->empty())
    {
        const bool serverSide = !mLevel->mIsClientSide;
        for (unsigned int i = 0; i < entities->size(); ++i)
        {
            boost::shared_ptr<Entity> entity = entities->at(i);
            if (entity->hasPassenger(shared_from_this()))
                continue;

            if (serverSide && getPassengers()->size() < 2 && !entity->isPassenger()
                && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                && !eTYPE_DERIVED_FROM(EntityType::WATER_ANIMAL, entity->GetType())
                && !eTYPE_DERIVED_FROM(EntityType::AGABLE_WATER_ANIMAL, entity->GetType())
                && !eTYPE_DERIVED_FROM(EntityType::TURTLE, entity->GetType())
                && !eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
                && !eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
            {
                entity->startRiding(shared_from_this());
            }
            else
            {
                push(entity);
            }
        }
    }

    PIXEndNamedEvent();
}

float Boat::getRowingTime(int paddle, float partialTicks)
{
    if (getPaddleState(paddle))
        return static_cast<float>(
            Mth::clampedLerp(mPaddlePositions[paddle] - 3.1415927f / 8.0f, mPaddlePositions[paddle], partialTicks));
    return 0.0f;
}

void Boat::clampRotation(boost::shared_ptr<Entity> passenger)
{
    passenger->setYBodyRot(mYRot);
    const float relativeYRot = Mth::wrapDegrees(passenger->mYRot - mYRot);
    const float clampedYRot = Mth::clamp(relativeYRot, -105.0f, 105.0f);
    passenger->mYRot += clampedYRot - relativeYRot;
    passenger->setYHeadRot(passenger->mYRot);
}

void Boat::positionRider(const boost::shared_ptr<Entity>& passenger)
{
    if (!hasPassenger(passenger))
        return;

    float offset = 0.0f;
    const float height = static_cast<float>((mRemoved ? 0.01f : getRideHeight()) + passenger->getRidingHeight());
    if (getPassengers()->size() > 1)
    {
        if (indexOf(*getPassengers(), passenger) == 0)
            offset = 0.2f;
        else
            offset = -0.6f;
        const bool isAnimal = eTYPE_DERIVED_FROM(EntityType::ANIMAL, passenger->GetType());
        if (isAnimal)
            offset += 0.2f;
    }

    const Vec3* seat = Vec3::newTemp(offset, 0.0, 0.0)->yRot(-mYRot * Mth::DEG_TO_RAD - 3.1415927f / 2.0f);
    passenger->setPos(mX + seat->x, mY + height, mZ + seat->z);
    passenger->mYRot += mDeltaRotation;
    passenger->setYHeadRot(passenger->getYHeadRot() + mDeltaRotation);
    clampRotation(passenger);

    const bool isAnimalPassenger = eTYPE_DERIVED_FROM(EntityType::ANIMAL, passenger->GetType());
    if (isAnimalPassenger && getPassengers()->size() > 1)
    {
        const int facing = (passenger->getId() & 1) == 0 ? 90 : 270;
        passenger->setYBodyRot(boost::static_pointer_cast<Animal>(passenger)->mYBodyRot + facing);
        passenger->setYHeadRot(passenger->getYHeadRot() + facing);
    }
}

void Boat::onPassengerTurned(boost::shared_ptr<Entity> passenger)
{
    clampRotation(passenger);
}

void Boat::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Type", getType());
    tag->putBoolean(L"Leashed", mLeashed);

    boost::shared_ptr<Entity> leashHolder = getLeashHolder();
    if (leashHolder)
    {
        CompoundTag* leashTag = new CompoundTag();
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, leashHolder->GetType()))
        {
            leashTag->putString(L"UUID", leashHolder->getUUID());
        }
        else if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, leashHolder->GetType()))
        {
            BlockPos pos = boost::dynamic_pointer_cast<HangingEntity>(leashHolder)->getPos();
            leashTag->putInt(L"X", pos.x);
            leashTag->putInt(L"Y", pos.y);
            leashTag->putInt(L"Z", pos.z);
        }
        tag->put(L"Leash", leashTag);
    }
}

void Boat::setType(Type type)
{
    mEntityData->set(DATA_TYPE, static_cast<int>(type), true);
}

void Boat::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"Type", Tag::TAG_INT))
        setType(static_cast<Type>(tag->getInt(L"Type")));

    mLeashed = tag->getBoolean(L"Leashed");
    if (mLeashed && tag->contains(L"Leash", Tag::TAG_COMPOUND))
        mLeashInfoTag = tag->getCompound(L"Leash")->copy();
}

bool Boat::testInteract(boost::shared_ptr<Player> player)
{
    return !player->isSneaking() && mOutOfControlTicks < 60 && !getIsSinking();
}

bool Boat::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInHand = player->getItemInHand(hand);
    if (itemInHand->getItem() == Items::lead)
    {
        if (!mLevel->mIsClientSide && isLeashedToPlayer(player))
        {
            dropLeash(true, !player->mAbilities.mInstabuild);
            return true;
        }
        if (canBeLeashed(player))
        {
            setLeashedTo(player, true);
            itemInHand->shrink(1);
            return true;
        }
    }

    if (player->isSneaking())
        return false;

    if (!mLevel->mIsClientSide && mOutOfControlTicks < 60 && !getIsSinking())
        player->startRiding(shared_from_this());

    if (!mLevel->mIsClientSide && isLeashedToPlayer(player))
    {
        dropLeash(true, !player->mAbilities.mInstabuild);
        return true;
    }

    return true;
}

unsigned int Boat::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (Item::getId(tooltipData.m_pItemInstance->getItem()) == ItemId::LEAD)
    {
        if (!isLeashed())
            return IDS_TOOLTIPS_LEASH;
        if (isLeashedToPlayer(tooltipData.m_pPlayer))
            return IDS_TOOLTIPS_UNLEASH;
    }

    if (getPassengers()->size() < 2 && testInteract(tooltipData.m_pPlayer))
        return IDS_TOOLTIPS_SAIL;

    return IDS_NONE;
}

unsigned int Boat::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToHurt ? IDS_TOOLTIPS_MINE : IDS_NONE;
}

void Boat::checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos)
{
    mLastYd = mYd;
    if (isPassenger())
        return;

    if (onGround)
    {
        if (mFallDistance > 3.0f && mStatus == ON_LAND)
        {
            causeFallDamage(mFallDistance, 1.0f);
            if (!mLevel->mIsClientSide && !mRemoved)
            {
                remove();
                if (mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
                {
                    for (int i = 0; i < 3; ++i)
                        spawnAtLocation(new ItemInstance(Item::byBlock(Blocks::planks), 1, getType()), 0.0f);
                    for (int i = 0; i < 2; ++i)
                        spawnAtLocation(Items::stick, 1, 0.0f);
                }
            }
        }
        mFallDistance = 0.0f;
    }
    else if (mLevel->getBlockState(mTickCache, BlockPos(this).below())->getMaterial() != Material::WATER && y < 0.0)
    {
        mFallDistance -= static_cast<float>(y);
    }
}

void Boat::setDamage(float damage)
{
    mEntityData->set(DATA_DAMAGE, damage, true);
}

float Boat::getDamage()
{
    return mEntityData->get(DATA_DAMAGE);
}

void Boat::setHurtTime(int time)
{
    mEntityData->set(DATA_HURT_TIME, time, true);
}

int Boat::getHurtTime()
{
    return mEntityData->get(DATA_HURT_TIME);
}

float Boat::getBubbleAngle(float partialTicks)
{
    return mBubbleAngleO + (mBubbleAngle - mBubbleAngleO) * partialTicks;
}

void Boat::setHurtDir(int direction)
{
    mEntityData->set(DATA_HURT_DIR, direction, true);
}

int Boat::getHurtDir()
{
    return mEntityData->get(DATA_HURT_DIR);
}

bool Boat::canAddPassenger(boost::shared_ptr<Entity> entity)
{
    return getPassengers()->size() < 2;
}

boost::shared_ptr<Entity> Boat::getControllingPassenger()
{
    const std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    if (passengers->empty())
        return boost::shared_ptr<Entity>();
    return passengers->at(0).lock();
}

void Boat::setInput(bool left, bool right, bool up, bool down)
{
    mInputLeft = left;
    mInputRight = right;
    mInputUp = up;
    mInputDown = down;
}

bool Boat::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return !isLeashed();
}

bool Boat::hasPlayerPassenger()
{
    const std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    for (unsigned int i = 0; i < passengers->size(); ++i)
    {
        const std::vector<boost::weak_ptr<Entity> >* current = getPassengers();
        if ((current->at(i).lock()->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
            return true;
    }
    return false;
}

void Boat::tickLeash()
{
    if (mLevel->mIsClientSide)
        return;

    Entity::tickLeash();
    if (isLeashed() && getLeashHolder() != NULL && getLeashHolder()->mLevel == mLevel)
    {
        boost::shared_ptr<Entity> holder = getLeashHolder();
        const float distance = distanceTo(holder);
        if (distance > 10.0f)
        {
            dropLeash(true, true);
            return;
        }

        if (!hasPlayerPassenger())
        {
            const Vec3* pos = getPos(0.0f);
            if (distance > 6.0f)
            {
                const double scale = 1.0f / distance;
                const double dx = (holder->getPos(0.0f)->x - pos->x) * scale;
                const double dy = (holder->getPos(0.0f)->y - pos->y) * scale;
                const double dz = (holder->getPos(0.0f)->z - pos->z) * scale;
                mXd += dx * std::fabs(dx) * 0.2f;
                mYd += dy * std::fabs(dy) * 0.2f;
                mZd += dz * std::fabs(dz) * 0.2f;
            }
        }
    }
}

unsigned long long Boat::GetType()
{
    return EntityType::BOAT;
}

Entity* Boat::create(Level* level)
{
    return new Boat(level);
}
