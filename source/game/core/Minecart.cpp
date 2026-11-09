#include "game/core/Minecart.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartFurnace.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/MinecartRideable.hpp"
#include "game/core/MinecartSpawner.hpp"
#include "game/core/MinecartTNT.hpp"
#include "game/core/Abilities.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/PushableBySelector.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PoweredRailBlock.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<float>* Minecart::DATA_ID_DAMAGE = NULL;
const EntityDataAccessor<int>* Minecart::DATA_ID_DISPLAY_BLOCK = NULL;
const EntityDataAccessor<int>* Minecart::DATA_ID_DISPLAY_OFFSET = NULL;
const EntityDataAccessor<int>* Minecart::DATA_ID_HURT_TIME = NULL;
const EntityDataAccessor<int>* Minecart::DATA_ID_HURT_DIR = NULL;
const EntityDataAccessor<bool>* Minecart::DATA_ID_CUSTOM_DISPLAY = NULL;

const int Minecart::EXITS[10][2][3] = {{{0, 0, -1}, {0, 0, 1}},  {{-1, 0, 0}, {1, 0, 0}},  {{-1, -1, 0}, {1, 0, 0}},
                                       {{-1, 0, 0}, {1, -1, 0}}, {{0, 0, -1}, {0, -1, 1}}, {{0, -1, -1}, {0, 0, 1}},
                                       {{0, 0, 1}, {1, 0, 0}},   {{0, 0, 1}, {-1, 0, 0}},  {{0, 0, -1}, {-1, 0, 0}},
                                       {{0, 0, -1}, {1, 0, 0}}};

std::wstring Minecart::Type::NAMES[7] = {L"MinecartRideable", L"MinecartChest",  L"MinecartFurnace",     L"MinecartTNT",
                                         L"MinecartSpawner",  L"MinecartHopper", L"MinecartCommandBlock"};

const Minecart::Type* Minecart::Type::RIDEABLE = new Minecart::Type(Minecart::Type::eType_Rideable);
const Minecart::Type* Minecart::Type::CHEST = new Minecart::Type(Minecart::Type::eType_Chest);
const Minecart::Type* Minecart::Type::FURNACE = new Minecart::Type(Minecart::Type::eType_Furnace);
const Minecart::Type* Minecart::Type::TNT = new Minecart::Type(Minecart::Type::eType_TNT);
const Minecart::Type* Minecart::Type::SPAWNER = new Minecart::Type(Minecart::Type::eType_Spawner);
const Minecart::Type* Minecart::Type::HOPPER = new Minecart::Type(Minecart::Type::eType_Hopper);
const Minecart::Type* Minecart::Type::COMMAND_BLOCK = new Minecart::Type(Minecart::Type::eType_CommandBlock);

const Minecart::Type* Minecart::Type::VALUES[7] = {
    Minecart::Type::RIDEABLE, Minecart::Type::CHEST,  Minecart::Type::FURNACE,      Minecart::Type::TNT,
    Minecart::Type::SPAWNER,  Minecart::Type::HOPPER, Minecart::Type::COMMAND_BLOCK};

Minecart::Minecart(Level* level) : Entity(level, true)
{
    _init();
}

Minecart::Minecart(Level* level, double x, double y, double z) : Entity(level, true)
{
    _init();
    setPos(x, y, z);
    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    mXo = x;
    mYo = y;
    mZo = z;

    int blockX = Mth::floor(x);
    int blockY = Mth::floor(y);
    int blockZ = Mth::floor(z);
    if (BaseRailBlock::isRail(level, BlockPos(blockX, blockY, blockZ)))
    {
        const BlockState* state = level->getBlockState(BlockPos(blockX, blockY, blockZ));
        BaseRailBlock* rail = static_cast<BaseRailBlock*>(state->getBlock());
        BaseRailBlock::RailShape* shape = state->getValue(rail->getShapeProperty());
        if (shape == BaseRailBlock::RailShape::NORTH_SOUTH || shape == BaseRailBlock::RailShape::ASCENDING_NORTH
            || shape == BaseRailBlock::RailShape::ASCENDING_SOUTH)
        {
            mYRotO = 90.0f;
            mYRot = 90.0f;
        }
    }
}

Minecart::~Minecart()
{
    operator delete(mReserved);
}

void Minecart::staticCtor()
{
    DATA_ID_HURT_TIME = SynchedEntityData::defineId<int>(EntityType::MINECART, EntityType::ENTITY,
                                                         EntityDataSerializers::INT);
    DATA_ID_HURT_DIR = SynchedEntityData::defineId<int>(EntityType::MINECART, EntityType::ENTITY,
                                                        EntityDataSerializers::INT);
    DATA_ID_DAMAGE = SynchedEntityData::defineId<float>(EntityType::MINECART, EntityType::ENTITY,
                                                        EntityDataSerializers::FLOAT);
    DATA_ID_DISPLAY_BLOCK = SynchedEntityData::defineId<int>(EntityType::MINECART, EntityType::ENTITY,
                                                             EntityDataSerializers::INT);
    DATA_ID_DISPLAY_OFFSET = SynchedEntityData::defineId<int>(EntityType::MINECART, EntityType::ENTITY,
                                                              EntityDataSerializers::INT);
    DATA_ID_CUSTOM_DISPLAY = SynchedEntityData::defineId<bool>(EntityType::MINECART, EntityType::ENTITY,
                                                               EntityDataSerializers::BOOLEAN);
}

void Minecart::_init()
{
    mBlocksBuilding = true;
    mLerpYd = 0.0;
    mLerpYRot = 0.0;
    mLerpXRot = 0.0;
    mLerpZd = 0.0;
    mLerpY = 0.0;
    mLerpXd = 0.0;
    mInBubbleColumn = false;
    mFlipped = false;
    mLerpX = 0.0;
    mLerpZ = 0.0;
    mLerpSteps = 0;
    setSize(0.98f, 0.7f);
    mReserved = NULL;
    mPushedMinecart = false;
    mCategories |= EntityCategory::eEntityCategory_Minecart;
}

void Minecart::setPos(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
    float halfWidth = mBbWidth / 2;
    float height = mBbHeight;
    setBoundingBox(AABB::newTemp(x - halfWidth, y, z - halfWidth, x + halfWidth, y + height, z + halfWidth));
}

boost::shared_ptr<Minecart> Minecart::createMinecart(Level* level, double x, double y, double z, const Type* type)
{
    switch (type->mId)
    {
    case Type::eType_Chest:
        return boost::shared_ptr<Minecart>(new MinecartChest(level, x, y, z));
    case Type::eType_Furnace:
        return boost::shared_ptr<Minecart>(new MinecartFurnace(level, x, y, z));
    case Type::eType_TNT:
        return boost::shared_ptr<Minecart>(new MinecartTNT(level, x, y, z));
    case Type::eType_Spawner:
        return boost::shared_ptr<Minecart>(new MinecartSpawner(level, x, y, z));
    case Type::eType_Hopper:
        return boost::shared_ptr<Minecart>(new MinecartHopper(level, x, y, z));
    default:
        return boost::shared_ptr<Minecart>(new MinecartRideable(level, x, y, z));
    }
}

bool Minecart::makeStepSound()
{
    return false;
}

void Minecart::defineSynchedData()
{
    mEntityData->createDateItem(DATA_ID_HURT_TIME, 0);
    mEntityData->createDateItem(DATA_ID_HURT_DIR, 1);
    mEntityData->createDateItem(DATA_ID_DAMAGE, 0.0f);
    mEntityData->createDateItem(DATA_ID_DISPLAY_BLOCK, 0);
    mEntityData->createDateItem(DATA_ID_DISPLAY_OFFSET, 6);
    mEntityData->createDateItem(DATA_ID_CUSTOM_DISPLAY, false);
}

AABB* Minecart::getCollideAgainstBox(boost::shared_ptr<Entity> entity)
{
    if (entity->isPushable(shared_from_this()))
    {
        return entity->mBoundingBox;
    }
    return NULL;
}

AABB* Minecart::getCollideBox()
{
    return NULL;
}

AABB* Minecart::getBoundingBoxForCulling()
{
    AABB* box = mBoundingBox;
    if (hasCustomDisplay())
    {
        int displayOffset = getDisplayOffset();
        return box->grow((displayOffset < 0 ? -displayOffset : displayOffset) * 0.0625);
    }
    return AABB::newTemp(box->mMinX, box->mMinY, box->mMinZ, box->mMaxX, box->mMaxY, box->mMaxZ);
}

bool Minecart::isPushable(boost::shared_ptr<Entity> entity)
{
    return true;
}

double Minecart::getRideHeight()
{
    return 0.0;
}

bool Minecart::hurt(const DamageSource* source, float damage)
{
    if (mLevel->mIsClientSide || mRemoved)
    {
        return true;
    }
    if (isInvulnerableTo(source))
    {
        return false;
    }

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
    markHurt();
    setDamage(getDamage() + damage * 10.0f);
    hasPassenger(source->getEntity());

    bool creativePlayer = source->getEntity() != NULL
                          && eTYPE_DERIVED_FROM(EntityType::PLAYER, source->getEntity()->GetType())
                          && boost::static_pointer_cast<Player>(source->getEntity())->mAbilities.mInstabuild;

    if (creativePlayer || getDamage() > 40.0f)
    {
        ejectPassengers();
        if (!creativePlayer || hasCustomName())
        {
            destroy(source);
        }
        else
        {
            remove();
        }
    }
    return true;
}

void Minecart::destroy(const DamageSource* source)
{
    remove();
    if (!mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
    {
        return;
    }

    not_null_ptr<ItemInstance> item(new ItemInstance(Items::minecart, 1));
    if (hasCustomName())
    {
        item->setHoverName(getCustomName());
    }
    spawnAtLocation(item, 0.0f);
}

void Minecart::animateHurt()
{
    setHurtDir(-getHurtDir());
    setHurtTime(10);
    setDamage(getDamage() + getDamage() * 10.0f);
}

bool Minecart::isPickable()
{
    return !mRemoved;
}

const Direction* Minecart::getMotionDirection()
{
    if (mFlipped)
    {
        return getDirection()->getOpposite()->getClockWise();
    }
    return getDirection()->getClockWise();
}

void Minecart::onAboveBubbleCol(bool down)
{
    if (!down && mLevel->getBlockState(BlockPos(mX, mY, mZ))->getMaterial() == Material::AIR)
    {
        mYd += 0.04f;
        return;
    }
    Entity::onAboveBubbleCol(down);
    mYd *= 0.8f;
}

bool Minecart::shouldTick()
{
    return true;
}

void Minecart::tick()
{
    PIXBeginNamedEvent(0.0f, "Minecart tick");
    for (int pass = 0; pass < 2; ++pass)
    {
        if (getHurtTime() > 0)
        {
            setHurtTime(getHurtTime() - 1);
        }
        if (getDamage() > 0.0f)
        {
            setDamage(getDamage() - 1.0f);
        }
        if (mY < -64.0)
        {
            outOfWorld();
        }

        if (!mLevel->mIsClientSide && dynamic_cast<ServerLevel*>(mLevel) != NULL)
        {
            MinecraftServer* server = mLevel->getServer();
            int waitTime = getPortalWaitTime();
            if (mIsInsidePortal)
            {
                if (server->isNetherEnabled())
                {
                    if (!isPassenger() && mPortalTime++ >= waitTime)
                    {
                        mPortalTime = waitTime;
                        mPortalCooldown = getDimensionChangingDelay();
                        int dimension = mLevel->mDimension->getType()->getId() == -1 ? 0 : -1;
                        changeDimension(dimension);
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
            if (mPortalCooldown > 0)
            {
                --mPortalCooldown;
            }
        }

        if (mLevel->mIsClientSide)
        {
            if (mLerpSteps > 0)
            {
                double x = mX + (mLerpX - mX) / mLerpSteps;
                double y = mY + (mLerpY - mY) / mLerpSteps;
                double z = mZ + (mLerpZ - mZ) / mLerpSteps;
                double yRotDelta = Mth::wrapDegrees(mLerpYRot - mYRot);
                mYRot += (float)(yRotDelta / mLerpSteps);
                mXRot = (float)(mXRot + (float)((mLerpXRot - mXRot) / mLerpSteps));
                --mLerpSteps;
                setPos(x, y, z);
                setRot(mYRot, mXRot);
            }
            else
            {
                setPos(mX, mY, mZ);
                setRot(mYRot, mXRot);
            }
            break;
        }

        mXo = mX;
        mYo = mY;
        mZo = mZ;
        if (!isNoGravity())
        {
            mYd -= 0.04f;
        }

        int x = Mth::floor(mX);
        int y = Mth::floor(mY);
        int z = Mth::floor(mZ);
        if (BaseRailBlock::isRail(mLevel, BlockPos(x, y - 1, z)))
        {
            --y;
        }

        BlockPos pos(x, y, z);
        const BlockState* state = mLevel->getBlockState(pos);
        if (BaseRailBlock::isRail(state))
        {
            mInBubbleColumn = false;
            moveAlongTrack(pos, state);
            if (state->getBlock() == Blocks::activator_rail)
            {
                activateMinecart(x, y, z, state->getValue(PoweredRailBlock::POWERED));
            }
        }
        else
        {
            BlockPos here(mX, mY, mZ);
            Material* material = mLevel->getBlockState(here)->getMaterial();
            Material* materialBelow = mLevel->getBlockState(here.below())->getMaterial();
            mInBubbleColumn = material == Material::BUBBLE_COLUMN
                              || (material == Material::AIR && materialBelow == Material::BUBBLE_COLUMN);
            comeOffTrack();
        }

        checkInsideBlocks();
        if (isInLava())
        {
            lavaHurt();
            mFallDistance *= 0.5f;
        }

        mXRot = 0.0f;
        double xDelta = mXo - mX;
        double zDelta = mZo - mZ;
        if (xDelta * xDelta + zDelta * zDelta > 0.001)
        {
            mYRot = (float)(std::atan2(zDelta, xDelta) * 180.0 / 3.1415927f);
            if (mFlipped)
            {
                mYRot += 180.0f;
            }
        }

        double rotDelta = Mth::wrapDegrees(mYRot - mYRotO);
        if (rotDelta < -170.0 || rotDelta >= 170.0)
        {
            mYRot += 180.0f;
            mFlipped = !mFlipped;
        }
        setRot(mYRot, mXRot);

        if (getType() == Type::RIDEABLE)
        {
            PushableBySelector selector(shared_from_this());
            std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
                shared_from_this(), mBoundingBox->grow(0.2f, 0.0, 0.2f), &selector, NULL);
            std::vector<boost::shared_ptr<Entity> >::iterator itEnd = entities->end();
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != itEnd; ++it)
            {
                boost::shared_ptr<Entity> entity = *it;
                if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
                    || eTYPE_DERIVED_FROM(EntityType::VILLAGER_GOLEM, entity->GetType())
                    || eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType())
                    || eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType()) || isVehicle() || entity->isPassenger())
                {
                    entity->push(shared_from_this());
                }
                else
                {
                    entity->startRiding(shared_from_this());
                }
            }
        }
        else
        {
            std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
                shared_from_this(), mBoundingBox->grow(0.2f, 0.0, 0.2f), NULL);
            std::vector<boost::shared_ptr<Entity> >::iterator itEnd = entities->end();
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != itEnd; ++it)
            {
                boost::shared_ptr<Entity> entity = *it;
                if (!hasPassenger(entity) && entity->isPushable(shared_from_this())
                    && eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
                {
                    boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
                    minecart->mPushedMinecart = false;
                    minecart->push(shared_from_this());
                    if (minecart->mPushedMinecart)
                    {
                        break;
                    }
                }
            }
        }
        updateInWaterState();
    }
    PIXEndNamedEvent();
}

double Minecart::getMaxSpeed()
{
    return 0.4;
}

void Minecart::activateMinecart(int x, int y, int z, bool powered)
{
}

void Minecart::comeOffTrack()
{
    double maxSpeed = getMaxSpeed();
    double xd = Mth::clamp(mXd, -maxSpeed, maxSpeed);
    double zd = Mth::clamp(mZd, -maxSpeed, maxSpeed);
    if (mOnGround)
    {
        xd *= 0.5;
        zd *= 0.5;
    }
    mXd = xd;
    mZd = zd;
    move(eMoverType_Self, mXd, mYd, mZd, false);
    if (!mOnGround && !mInBubbleColumn)
    {
        mXd *= 0.95f;
        mZd *= 0.95f;
    }
}

void Minecart::moveAlongTrack(const BlockPos& pos, const BlockState* state)
{
    mFallDistance = 0.0f;
    Vec3* oldPos = getPos(mX, mY, mZ);
    mY = pos.y;

    bool powered = false;
    bool braking = false;
    BaseRailBlock* rail = dynamic_cast<BaseRailBlock*>(state->getBlock());
    if (rail == Blocks::golden_rail)
    {
        powered = state->getValue(PoweredRailBlock::POWERED);
        braking = !powered;
    }

    const double slopeSpeed = 0.0078125;
    BaseRailBlock::RailShape* shape = state->getValue(rail->getShapeProperty());
    switch (shape->getData())
    {
    case 2:
        mXd -= slopeSpeed;
        ++mY;
        break;
    case 3:
        mXd += slopeSpeed;
        ++mY;
        break;
    case 4:
        mZd += slopeSpeed;
        ++mY;
        break;
    case 5:
        mZd -= slopeSpeed;
        ++mY;
        break;
    default:
        break;
    }

    int exits[2][3];
    std::memcpy(exits, EXITS[shape->getData()], sizeof(exits));
    double xDir = exits[1][0] - exits[0][0];
    double zDir = exits[1][2] - exits[0][2];
    double length = std::sqrt(xDir * xDir + zDir * zDir);
    if (mXd * xDir + mZd * zDir < 0.0)
    {
        xDir = -xDir;
        zDir = -zDir;
    }

    double speed = std::sqrt(mXd * mXd + mZd * mZd);
    if (speed > 2.0)
    {
        speed = 2.0;
    }
    mXd = speed * xDir / length;
    mZd = speed * zDir / length;

    std::vector<boost::weak_ptr<Entity> >* passengers = getPassengers();
    if (!passengers->empty())
    {
        boost::shared_ptr<Entity> passenger = passengers->at(0).lock();
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, passenger->GetType()))
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(passenger);
            if (living->mZza > 0.0 && mXd * mXd + mZd * mZd < 0.01)
            {
                float xPush = -std::sin(living->mYRot * 0.017453292f);
                float zPush = std::cos(living->mYRot * 0.017453292f);
                mXd += xPush * 0.1;
                mZd += zPush * 0.1;
                braking = false;
            }
        }
    }

    if (braking)
    {
        double brakeSpeed = std::sqrt(mXd * mXd + mZd * mZd);
        if (brakeSpeed < 0.03)
        {
            mXd = 0.0;
            mYd = 0.0;
            mZd = 0.0;
        }
        else
        {
            mXd *= 0.5;
            mYd *= 0.0;
            mZd *= 0.5;
        }
    }

    double x0 = pos.x + exits[0][0] * 0.5 + 0.5;
    double z0 = pos.z + exits[0][2] * 0.5 + 0.5;
    double x1 = pos.x + exits[1][0] * 0.5 + 0.5;
    double z1 = pos.z + exits[1][2] * 0.5 + 0.5;
    xDir = x1 - x0;
    zDir = z1 - z0;
    double progress;
    if (xDir == 0.0)
    {
        progress = mZ - pos.z;
    }
    else if (zDir == 0.0)
    {
        progress = mX - pos.x;
    }
    else
    {
        progress = ((mX - x0) * xDir + (mZ - z0) * zDir) * 2.0;
    }
    mX = x0 + xDir * progress;
    mZ = z0 + zDir * progress;
    setPos(mX, mY, mZ);

    double xd = mXd;
    double zd = mZd;
    if (isVehicle())
    {
        xd *= 0.75;
        zd *= 0.75;
    }
    double maxSpeed = getMaxSpeed();
    move(eMoverType_Self, Mth::clamp(xd, -maxSpeed, maxSpeed), 0.0, Mth::clamp(zd, -maxSpeed, maxSpeed), false);

    if (exits[0][1] != 0 && Mth::floor(mX) - pos.x == exits[0][0] && Mth::floor(mZ) - pos.z == exits[0][2])
    {
        setPos(mX, mY + exits[0][1], mZ);
    }
    else if (exits[1][1] != 0 && Mth::floor(mX) - pos.x == exits[1][0] && Mth::floor(mZ) - pos.z == exits[1][2])
    {
        setPos(mX, mY + exits[1][1], mZ);
    }

    applyNaturalSlowdown();

    Vec3* newPos = getPos(mX, mY, mZ);
    if (newPos != NULL && oldPos != NULL)
    {
        double slopeBoost = (oldPos->y - newPos->y) * 0.05;
        double currentSpeed = std::sqrt(mXd * mXd + mZd * mZd);
        if (currentSpeed > 0.0)
        {
            mXd = mXd / currentSpeed * (currentSpeed + slopeBoost);
            mZd = mZd / currentSpeed * (currentSpeed + slopeBoost);
        }
        setPos(mX, newPos->y, mZ);
    }

    int xBlock = Mth::floor(mX);
    int zBlock = Mth::floor(mZ);
    if (xBlock != pos.x || zBlock != pos.z)
    {
        double currentSpeed = std::sqrt(mXd * mXd + mZd * mZd);
        mXd = currentSpeed * (xBlock - pos.x);
        mZd = currentSpeed * (zBlock - pos.z);
    }

    if (powered)
    {
        double currentSpeed = std::sqrt(mXd * mXd + mZd * mZd);
        if (currentSpeed > 0.01)
        {
            const double boost = 0.06;
            mXd += mXd / currentSpeed * boost;
            mZd += mZd / currentSpeed * boost;
        }
        else if (shape == BaseRailBlock::RailShape::EAST_WEST)
        {
            if (mLevel->getBlockState(pos.west())->isSolidBlockingCubeAndNotSignalSource())
            {
                mXd = 0.02;
            }
            else if (mLevel->getBlockState(pos.east())->isSolidBlockingCubeAndNotSignalSource())
            {
                mXd = -0.02;
            }
        }
        else if (shape == BaseRailBlock::RailShape::NORTH_SOUTH)
        {
            if (mLevel->getBlockState(pos.north())->isSolidBlockingCubeAndNotSignalSource())
            {
                mZd = 0.02;
            }
            else if (mLevel->getBlockState(pos.south())->isSolidBlockingCubeAndNotSignalSource())
            {
                mZd = -0.02;
            }
        }
    }
}

void Minecart::applyNaturalSlowdown()
{
    if (isVehicle())
    {
        mXd *= 0.997f;
        mYd *= 0.0;
        mZd *= 0.997f;
    }
    else
    {
        mXd *= 0.96f;
        mYd *= 0.0;
        mZd *= 0.96f;
    }
}

Vec3* Minecart::getPosOffs(double x, double y, double z, double distance)
{
    int xBlock = Mth::floor(x);
    int yBlock = Mth::floor(y);
    int zBlock = Mth::floor(z);
    if (BaseRailBlock::isRail(mLevel, BlockPos(xBlock, yBlock - 1, zBlock)))
    {
        --yBlock;
    }

    const BlockState* state = mLevel->getBlockState(BlockPos(xBlock, yBlock, zBlock));
    if (BaseRailBlock::isRail(state))
    {
        BaseRailBlock* rail = dynamic_cast<BaseRailBlock*>(state->getBlock());
        BaseRailBlock::RailShape* shape = state->getValue(rail->getShapeProperty());
        y = yBlock;
        if (shape->isAscending())
        {
            y = yBlock + 1;
        }

        int exits[2][3];
        std::memcpy(exits, EXITS[shape->getData()], sizeof(exits));
        double xDir = exits[1][0] - exits[0][0];
        double zDir = exits[1][2] - exits[0][2];
        double length = std::sqrt(xDir * xDir + zDir * zDir);
        xDir /= length;
        zDir /= length;
        x += xDir * distance;
        z += zDir * distance;

        if (exits[0][1] != 0 && Mth::floor(x) - xBlock == exits[0][0] && Mth::floor(z) - zBlock == exits[0][2])
        {
            y += exits[0][1];
        }
        else if (exits[1][1] != 0 && Mth::floor(x) - xBlock == exits[1][0] && Mth::floor(z) - zBlock == exits[1][2])
        {
            y += exits[1][1];
        }
        return getPos(x, y, z);
    }
    return NULL;
}

Vec3* Minecart::getPos(double x, double y, double z)
{
    int xBlock = Mth::floor(x);
    int yBlock = Mth::floor(y);
    int zBlock = Mth::floor(z);
    if (BaseRailBlock::isRail(mLevel, BlockPos(xBlock, yBlock - 1, zBlock)))
    {
        --yBlock;
    }

    const BlockState* state = mLevel->getBlockState(BlockPos(xBlock, yBlock, zBlock));
    if (BaseRailBlock::isRail(state))
    {
        BaseRailBlock* rail = dynamic_cast<BaseRailBlock*>(state->getBlock());
        BaseRailBlock::RailShape* shape = state->getValue(rail->getShapeProperty());
        int exits[2][3];
        std::memcpy(exits, EXITS[shape->getData()], sizeof(exits));

        double x0 = xBlock + 0.5 + exits[0][0] * 0.5;
        double y0 = yBlock + 0.0625 + exits[0][1] * 0.5;
        double z0 = zBlock + 0.5 + exits[0][2] * 0.5;
        double x1 = xBlock + 0.5 + exits[1][0] * 0.5;
        double y1 = yBlock + 0.0625 + exits[1][1] * 0.5;
        double z1 = zBlock + 0.5 + exits[1][2] * 0.5;
        double xDir = x1 - x0;
        double yDir = (y1 - y0) * 2.0;
        double zDir = z1 - z0;

        double progress;
        if (xDir == 0.0)
        {
            progress = z - zBlock;
        }
        else if (zDir == 0.0)
        {
            progress = x - xBlock;
        }
        else
        {
            double xOffset = x - x0;
            double zOffset = z - z0;
            progress = (xOffset * xDir + zOffset * zDir) * 2.0;
        }

        x = x0 + xDir * progress;
        y = y0 + yDir * progress;
        z = z0 + zDir * progress;
        if (yDir < 0.0)
        {
            y += 1.0;
        }
        if (yDir > 0.0)
        {
            y += 0.5;
        }
        return Vec3::newTemp(x, y, z);
    }
    return NULL;
}

void Minecart::addAdditonalSaveData(CompoundTag* tag)
{
    if (hasCustomDisplay())
    {
        tag->putBoolean(L"CustomDisplayTile", true);
        const BlockState* state = getDisplayBlockState();
        ResourceLocation location = Block::blockRegistry->getKey(state->getBlock());
        tag->putString(L"DisplayTile", location.toString());
        tag->putInt(L"DisplayData", state->getBlock()->convertBlockStateToLegacyData(state));
        tag->putInt(L"DisplayOffset", getDisplayOffset());
    }
}

void Minecart::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->getBoolean(L"CustomDisplayTile"))
    {
        Block* block;
        if (tag->contains(L"DisplayTile", Tag::TAG_STRING))
        {
            block = Block::byString(tag->getString(L"DisplayTile"));
        }
        else
        {
            block = Block::byId(tag->getInt(L"DisplayTile"));
        }

        int data = tag->getInt(L"DisplayData");
        setDisplayBlockState(block == NULL ? Blocks::air->defaultBlockState() : block->getBlockState(data));
        setDisplayOffset(tag->getInt(L"DisplayOffset"));
    }
}

void Minecart::push(boost::shared_ptr<Entity> entity)
{
    if (mLevel->mIsClientSide)
    {
        return;
    }
    if (entity->mNoPhysics || mNoPhysics)
    {
        return;
    }
    if (hasPassenger(entity))
    {
        return;
    }
    for (unsigned int i = 0; i < mPassengers.size(); ++i)
    {
        boost::shared_ptr<Entity> passenger = mPassengers.at(i).lock();
        if (passenger != NULL && passenger->hasPassenger(entity))
        {
            return;
        }
    }

    double xDistance = entity->mX - mX;
    double zDistance = entity->mZ - mZ;
    double distanceSq = xDistance * xDistance + zDistance * zDistance;
    if (distanceSq < 0.0001)
    {
        return;
    }

    double distance = std::sqrt(distanceSq);
    xDistance /= distance;
    zDistance /= distance;
    double scale = 1.0 / distance;
    if (scale > 1.0)
    {
        scale = 1.0;
    }
    xDistance *= scale;
    zDistance *= scale;
    xDistance *= 0.1f;
    zDistance *= 0.1f;
    xDistance *= 1.0f - mPushthrough;
    zDistance *= 1.0f - mPushthrough;
    xDistance *= 0.5;
    zDistance *= 0.5;

    if (eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
    {
        double xOffset = entity->mX - mX;
        double zOffset = entity->mZ - mZ;
        Vec3* toEntity = Vec3::newTemp(xOffset, 0.0, zOffset)->normalize();
        Vec3* facing = Vec3::newTemp(std::cos(mYRot * 0.017453292f), 0.0, std::sin(mYRot * 0.017453292f))->normalize();
        if (std::abs(toEntity->dot(facing)) < 0.8)
        {
            return;
        }

        double xd = entity->mXd + mXd;
        double zd = entity->mZd + mZd;
        boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
        if (minecart != NULL && minecart->getType() == Type::FURNACE && getType() != Type::FURNACE)
        {
            mXd *= 0.2f;
            mZd *= 0.2f;
            push(entity->mXd - xDistance, 0.0, entity->mZd - zDistance);
            entity->mXd *= 0.95f;
            entity->mZd *= 0.95f;
            mPushedMinecart = true;
        }
        else if (minecart != NULL && minecart->getType() != Type::FURNACE && getType() == Type::FURNACE)
        {
            entity->mXd *= 0.2f;
            entity->mZd *= 0.2f;
            entity->push(mXd + xDistance, 0.0, mZd + zDistance);
            mXd *= 0.95f;
            mZd *= 0.95f;
            mPushedMinecart = true;
        }
        else
        {
            xd *= 0.5;
            zd *= 0.5;
            mXd *= 0.2f;
            mZd *= 0.2f;
            push(xd - xDistance, 0.0, zd - zDistance);
            entity->mXd *= 0.2f;
            entity->mZd *= 0.2f;
            entity->push(xd + xDistance, 0.0, zd + zDistance);
            mPushedMinecart = true;

            if (std::abs(xOffset) < 1.0 && std::abs(zOffset) < 1.0)
            {
                double separation = 2.0 - (std::abs(xOffset) + std::abs(zOffset)) * 0.5;
                entity->mXd /= separation;
                entity->mZd /= separation;
            }
            if (!(mXd == mXd))
            {
                mXd = 0.0;
            }
            if (!(mZd == mZd))
            {
                mZd = 0.0;
            }
            if (!(entity->mXd == entity->mXd))
            {
                entity->mXd = 0.0;
            }
            if (!(entity->mZd == entity->mZd))
            {
                entity->mZd = 0.0;
            }
        }
    }
    else
    {
        push(-xDistance, 0.0, -zDistance);
        entity->push(xDistance * 0.25, 0.0, zDistance * 0.25);
    }
}

bool Minecart::isInterpolating() const
{
    return mLerpSteps > 0;
}

double Minecart::getLerpToYRot() const
{
    return mLerpYRot;
}

double Minecart::getLerpToXRot() const
{
    return mLerpXRot;
}

void Minecart::lerpTo(double x, double y, double z, float yRot, float xRot, int steps, bool teleport)
{
    if (mLevel->mIsClientSide)
    {
        y = Mth::ceil(y, 0.1);
    }
    mLerpX = x;
    mLerpY = y;
    mLerpZ = z;
    mLerpYRot = yRot;
    mLerpXRot = xRot;
    mLerpSteps = steps + 2;
    mXd = mLerpXd;
    mYd = mLerpYd;
    mZd = mLerpZd;
}

void Minecart::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    mLerpXd = xd;
    mLerpYd = yd;
    mLerpZd = zd;
}

void Minecart::setDamage(float damage)
{
    mEntityData->set(DATA_ID_DAMAGE, damage, true);
}

float Minecart::getDamage()
{
    return mEntityData->get(DATA_ID_DAMAGE);
}

void Minecart::setHurtTime(int hurtTime)
{
    mEntityData->set(DATA_ID_HURT_TIME, hurtTime, true);
}

int Minecart::getHurtTime()
{
    return mEntityData->get(DATA_ID_HURT_TIME);
}

void Minecart::setHurtDir(int hurtDir)
{
    mEntityData->set(DATA_ID_HURT_DIR, hurtDir, true);
}

int Minecart::getHurtDir()
{
    return mEntityData->get(DATA_ID_HURT_DIR);
}

const BlockState* Minecart::getDisplayBlockState()
{
    if (!hasCustomDisplay())
    {
        return getDefaultDisplayBlockState();
    }
    return Block::stateByIdWithData(getEntityData()->get(DATA_ID_DISPLAY_BLOCK));
}

const BlockState* Minecart::getDefaultDisplayBlockState()
{
    return Blocks::air->defaultBlockState();
}

int Minecart::getDisplayOffset()
{
    if (!hasCustomDisplay())
    {
        return getDefaultDisplayOffset();
    }
    return getEntityData()->get(DATA_ID_DISPLAY_OFFSET);
}

int Minecart::getDefaultDisplayOffset()
{
    return 6;
}

void Minecart::setDisplayBlockState(const BlockState* state)
{
    getEntityData()->set(DATA_ID_DISPLAY_BLOCK, Block::getIdWithData(state), true);
    setCustomDisplay(true);
}

void Minecart::setDisplayOffset(int offset)
{
    getEntityData()->set(DATA_ID_DISPLAY_OFFSET, offset, true);
    setCustomDisplay(true);
}

bool Minecart::hasCustomDisplay()
{
    return getEntityData()->get(DATA_ID_CUSTOM_DISPLAY);
}

void Minecart::setCustomDisplay(bool customDisplay)
{
    getEntityData()->set(DATA_ID_CUSTOM_DISPLAY, customDisplay, true);
}

unsigned int Minecart::GetHitTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToHurt ? IDS_TOOLTIPS_MINE : IDS_NONE;
}

boost::shared_ptr<Entity> Minecart::getControllingPassenger()
{
    return boost::shared_ptr<Entity>();
}

unsigned long long Minecart::GetType()
{
    return EntityType::MINECART;
}

std::wstring Minecart::Type::getName() const
{
    return NAMES[mId];
}
