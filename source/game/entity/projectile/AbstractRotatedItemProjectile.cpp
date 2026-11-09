#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Inventory.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/generation/Player.hpp"

bool sArrowTargetsPredicatesFunc(boost::shared_ptr<Entity> entity)
{
    return entity->isPickable();
}

static Predicate<boost::shared_ptr<Entity> >* sArrowTargetsPredicates =
    new Predicates<boost::shared_ptr<Entity> >::FunctionPredicate(sArrowTargetsPredicatesFunc);

void fjSanitisePosAndVel(Entity* entity)
{
    entity->mX = static_cast<int>(std::floor(entity->mX * 32.0)) * 0.03125;
    entity->mY = static_cast<int>(std::floor(entity->mY * 32.0)) * 0.03125;
    entity->mZ = static_cast<int>(std::floor(entity->mZ * 32.0)) * 0.03125;

    entity->mXd = static_cast<int>(Mth::clamp(entity->mXd, -3.9, 3.9) * 8000.0) / 8000.0;
    entity->mYd = static_cast<int>(Mth::clamp(entity->mYd, -3.9, 3.9) * 8000.0) / 8000.0;
    entity->mZd = static_cast<int>(Mth::clamp(entity->mZd, -3.9, 3.9) * 8000.0) / 8000.0;
}

const EntityDataAccessor<unsigned char>* AbstractRotatedItemProjectile::DATA_FLAGS = NULL;

AbstractRotatedItemProjectile::AbstractRotatedItemProjectile(Level* level)
    : Entity(level, true),
      Projectile(level, boost::shared_ptr<LivingEntity>())
{
    _init();
}

AbstractRotatedItemProjectile::AbstractRotatedItemProjectile(Level* level, double x, double y, double z)
    : Entity(level, true),
      Projectile(level, boost::shared_ptr<LivingEntity>())
{
    _init();
    setPos(x, y, z);
}

AbstractRotatedItemProjectile::AbstractRotatedItemProjectile(Level* level, boost::shared_ptr<LivingEntity> owner)
    : Entity(level, true),
      Projectile(level, owner)
{
    _init();
    setPos(owner->mX, owner->mY + owner->getEyeHeight() - 0.10000000149011612, owner->mZ);
    mOwner = owner;
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, owner->GetType()))
        mPickup = ePickup_Allowed;
}

void AbstractRotatedItemProjectile::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity)
{
    (void)entity;
}

void AbstractRotatedItemProjectile::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId(EntityType::ARROW, EntityType::ENTITY, EntityDataSerializers::BYTE);
}

void AbstractRotatedItemProjectile::_init()
{
    mYRotO = 0.0f;
    mYRot = 0.0f;
    mInTile = Blocks::air;
    mZTile = -1;
    mInData = 0;
    mYTile = -1;
    mInGround = false;
    mShake = 0;
    mPickup = ePickup_Disallowed;
    mLife = 0;
    mXTile = -1;
    mInGroundTime = 0;
    mFlightTime = 0;
    setSize(0.5f, 0.5f);
    mClampedToLevel = false;
}

void AbstractRotatedItemProjectile::defineSynchedData()
{
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
}

void AbstractRotatedItemProjectile::shoot(double x, double y, double z, float velocity, float inaccuracy)
{
    float length = Mth::sqrt(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;
    x += mRandom->nextGaussian() * 0.0075 * inaccuracy;
    y += mRandom->nextGaussian() * 0.0075 * inaccuracy;
    z += mRandom->nextGaussian() * 0.0075 * inaccuracy;
    x *= velocity;
    y *= velocity;
    z *= velocity;
    mXd = x;
    mYd = y;
    mZd = z;
    double horizontal = std::sqrt(x * x + z * z);
    mYRot = static_cast<float>(std::atan2(x, z)) * Mth::RAD_TO_DEG;
    mXRot = static_cast<float>(std::atan2(y, horizontal)) * Mth::RAD_TO_DEG;
    mYRotO = mYRot;
    mXRotO = mXRot;
    mLife = 0;
    fjSanitisePosAndVel(this);
}

void AbstractRotatedItemProjectile::shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot,
                                                      float offset, float velocity, float inaccuracy)
{
    float x = -std::sin(yRot * Mth::DEG_TO_RAD) * std::cos(xRot * Mth::DEG_TO_RAD);
    float y = -std::sin(xRot * Mth::DEG_TO_RAD);
    float z = std::cos(yRot * Mth::DEG_TO_RAD) * std::cos(xRot * Mth::DEG_TO_RAD);
    shoot(x, y, z, velocity, inaccuracy);
    mXd += entity->mXd;
    mZd += entity->mZd;
    if (!entity->mOnGround)
        mYd += entity->mYd;
    fjSanitisePosAndVel(this);
}

void AbstractRotatedItemProjectile::setFlag(int flag, bool value)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    if (value)
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags | flag), true);
    else
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags & ~flag), true);
}

void AbstractRotatedItemProjectile::setCritArrow(bool critical)
{
    setFlag(1, critical);
}

bool AbstractRotatedItemProjectile::onHit(const HitResult* result)
{
    boost::shared_ptr<Entity> hitEntity = result->mEntity;
    if (hitEntity != NULL)
    {
        onHitEntity(result);
        return true;
    }
    else
    {
        BlockPos pos = result->getBlockPos();
        mXTile = pos.x;
        mYTile = pos.y;
        mZTile = pos.z;
        const BlockState* state = mLevel->getBlockState(pos);
        mInTile = state->getBlock();
        mInData = mInTile->convertBlockStateToLegacyData(state);
        mXd = result->mLocation->x - mX;
        mYd = result->mLocation->y - mY;
        mZd = result->mLocation->z - mZ;
        double length = std::sqrt(mXd * mXd + mYd * mYd + mZd * mZd);
        if (length != 0.0)
        {
            double offsetX = mXd / length;
            if (offsetX == offsetX)
                mX -= offsetX * 0.05;
            double offsetY = mYd / length;
            if (offsetY == offsetY)
                mY -= offsetY * 0.05;
            double offsetZ = mZd / length;
            if (offsetZ == offsetZ)
                mZ -= offsetZ * 0.05;
        }
        playSound(getOnHitSound(), 1.0f, 1.2f / (mRandom->nextFloat() * 0.2f + 0.9f));
        mInGround = true;
        mShake = 7;
        setCritArrow(false);
        if (state->getMaterial() != Material::AIR)
        {
            mInTile->entityInside(mLevel, pos, state, shared_from_this());
            return true;
        }
        return false;
    }
}

const SoundEvent* AbstractRotatedItemProjectile::getOnHitSound() const
{
    return SoundEvents::ARROW_HIT;
}

boost::shared_ptr<Entity> AbstractRotatedItemProjectile::findHitEntity(Vec3* from, Vec3* to)
{
    boost::shared_ptr<Entity> hitEntity;
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
        shared_from_this(), mBoundingBox->expand(mXd, mYd, mZd)->grow(1.0), sArrowTargetsPredicates, NULL);
    double nearestDistance = 0.0;
    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> entity = entities->at(i);
        if (entity == mOwner && mFlightTime < 5)
            continue;
        HitResult* hit = entity->mBoundingBox->grow(0.3)->clip(from, to);
        if (hit != NULL)
        {
            double distance = from->distanceToSqr(hit->mLocation);
            if (distance < nearestDistance || nearestDistance == 0.0)
            {
                hitEntity = entity;
                nearestDistance = distance;
            }
        }
    }
    return hitEntity;
}

void AbstractRotatedItemProjectile::checkDespawn()
{
    if (++mLife >= 1200)
        remove();
}

bool AbstractRotatedItemProjectile::makeStepSound()
{
    return false;
}

float AbstractRotatedItemProjectile::getWaterInertia()
{
    return 0.6f;
}

bool AbstractRotatedItemProjectile::canChangeDimensions()
{
    return false;
}

bool AbstractRotatedItemProjectile::shouldRenderAtSqrDistance(double distanceSqr)
{
    double distance = mBoundingBox->getSize() * 10.0;
    if (Mth::isNaN(distance))
        distance = 1.0;
    distance *= 64.0 * getViewScale();
    return distanceSqr < distance * distance;
}

bool AbstractRotatedItemProjectile::isNoPhysics()
{
    if (!mLevel->mIsClientSide)
        return mNoPhysics;
    return (mEntityData->get(DATA_FLAGS) >> 1 & 1) != 0;
}

bool AbstractRotatedItemProjectile::isCritArrow()
{
    return (mEntityData->get(DATA_FLAGS) & 1) != 0;
}

void AbstractRotatedItemProjectile::tick()
{
    Entity::tick();
    if (mXRotO == 0.0f && mYRotO == 0.0f)
    {
        float horizontal = Mth::sqrt(mXd * mXd + mZd * mZd);
        mYRot = static_cast<float>(std::atan2(mXd, mZd)) * Mth::RAD_TO_DEG;
        mXRot = atan2f(static_cast<float>(mYd), horizontal) * Mth::RAD_TO_DEG;
        mYRotO = mYRot;
        mXRotO = mXRot;
    }

    BlockPos tilePos(mXTile, mYTile, mZTile);
    const BlockState* state = mLevel->getBlockState(tilePos);
    Block* block = state->getBlock();
    if (state->getMaterial() != Material::AIR && !isNoPhysics())
    {
        const AABB* shape = state->getClipAABB(mLevel, tilePos);
        if (shape != Block::NULL_AABB && shape->move(tilePos)->contains(Vec3::newTemp(mX, mY, mZ)))
            mInGround = true;
    }

    if (mShake > 0)
        --mShake;

    if (mInGround && !isNoPhysics())
    {
        int data = block->convertBlockStateToLegacyData(state);
        if ((block != mInTile || data != mInData) && !mLevel->hasBlockCubes(mBoundingBox->grow(0.05)))
        {
            mInGround = false;
            mXd *= mRandom->nextDouble() * 0.2;
            mYd *= mRandom->nextDouble() * 0.2;
            mZd *= mRandom->nextDouble() * 0.2;
            mLife = 0;
            mFlightTime = 0;
            ++mInGroundTime;
            return;
        }
        checkDespawn();
        ++mInGroundTime;
        return;
    }

    mInGroundTime = 0;
    ++mFlightTime;
    HitResult* hitResult = mLevel->clip(Vec3::newTemp(mX, mY, mZ), Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd), false,
                                        true, false, false, NULL, false);
    Vec3* from = Vec3::newTemp(mX, mY, mZ);
    Vec3* to = hitResult == NULL
                   ? Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd)
                   : Vec3::newTemp(hitResult->mLocation->x, hitResult->mLocation->y, hitResult->mLocation->z);

    boost::shared_ptr<Entity> hitEntity = findHitEntity(from, to);
    if (hitEntity)
    {
        delete hitResult;
        hitResult = new HitResult(hitEntity);
    }

    if (hitResult != NULL && hitResult->mType == HitResult::ENTITY
        && eTYPE_DERIVED_FROM(EntityType::PLAYER, hitResult->mEntity->GetType()))
    {
        boost::shared_ptr<Player> hitPlayer = boost::static_pointer_cast<Player>(hitResult->mEntity);
        if (mOwner && eTYPE_DERIVED_FROM(EntityType::PLAYER, mOwner->GetType())
            && !boost::static_pointer_cast<Player>(mOwner)->canHarmPlayer(hitPlayer))
        {
            delete hitResult;
            hitResult = NULL;
        }
    }

    if (hitResult != NULL)
    {
        if (!isNoPhysics())
        {
            onHit(hitResult);
            delete hitResult;
        }
    }

    if (isCritArrow())
    {
        for (int i = 0; i < 4; ++i)
        {
            mLevel->addParticle(PARTICLE_CRIT, mX + mXd * i / 4.0, mY + mYd * i / 4.0, mZ + mZd * i / 4.0, -mXd,
                                0.2 - mYd, -mZd, arrayWithLength<int>());
        }
    }

    mX += mXd;
    mY += mYd;
    mZ += mZd;
    float horizontal = Mth::sqrt(mXd * mXd + mZd * mZd);
    if (isNoPhysics())
        mYRot = static_cast<float>(std::atan2(-mXd, -mZd)) * Mth::RAD_TO_DEG;
    else
        mYRot = static_cast<float>(std::atan2(mXd, mZd)) * Mth::RAD_TO_DEG;
    float xRot = atan2f(static_cast<float>(mYd), horizontal) * Mth::RAD_TO_DEG;

    while (xRot - mXRotO < -180.0f)
        mXRotO -= 360.0f;
    while (xRot - mXRotO >= 180.0f)
        mXRotO += 360.0f;
    while (mYRot - mYRotO < -180.0f)
        mYRotO -= 360.0f;
    while (mYRot - mYRotO >= 180.0f)
        mYRotO += 360.0f;

    mXRot = mXRotO + (xRot - mXRotO) * 0.2f;
    mYRot = mYRotO + (mYRot - mYRotO) * 0.2f;
    float inertia = 0.99f;
    if (isInWater())
    {
        for (int i = 0; i < 4; ++i)
            mLevel->addParticle(PARTICLE_BUBBLE, mX - mXd * 0.25, mY - mYd * 0.25, mZ - mZd * 0.25, mXd, mYd, mZd,
                                arrayWithLength<int>());
        inertia = getWaterInertia();
    }
    if (isInWaterOrRain())
        clearFire();
    mXd *= inertia;
    mYd *= inertia;
    mZd *= inertia;
    if (!isNoGravity() && !isNoPhysics())
        mYd -= 0.05000000074505806;
    setPos(mX, mY, mZ);
    checkInsideBlocks();

    if (mClampedToLevel)
    {
        double x = mX;
        double y = mY;
        double z = mZ;
        mLevel->clampWithinLevelBounds(x, z, 0.0);
        mLevel->clampWithinLevelHeight(y, mBbHeight);
        if (x != mX || z != mZ || y != mY)
        {
            setPos(x, y, z);
            mInGround = true;
        }
    }
}

void AbstractRotatedItemProjectile::lerpTo(double x, double y, double z, float yRot, float xRot, int steps,
                                           bool teleport)
{
    setPos(x, y, z);
    setRot(yRot, xRot);
}

void AbstractRotatedItemProjectile::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRotO == 0.0f && mYRotO == 0.0f)
    {
        float horizontal = Mth::sqrt(xd * xd + zd * zd);
        mXRot = atan2f(static_cast<float>(yd), horizontal) * Mth::RAD_TO_DEG;
        mYRot = static_cast<float>(std::atan2(xd, zd)) * Mth::RAD_TO_DEG;
        mXRotO = mXRot;
        mYRotO = mYRot;
        moveTo(mX, mY, mZ, mYRot, mXRot);
        mLife = 0;
    }
}

void AbstractRotatedItemProjectile::move(MoverType type, double x, double y, double z, bool pushedByPiston)
{
    Entity::move(type, x, y, z, pushedByPiston);
    if (mInGround)
    {
        mXTile = Mth::floor(mX);
        mYTile = Mth::floor(mY);
        mZTile = Mth::floor(mZ);
    }
}

void AbstractRotatedItemProjectile::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"xTile", mXTile);
    tag->putInt(L"yTile", mYTile);
    tag->putInt(L"zTile", mZTile);
    tag->putShort(L"life", static_cast<short>(mLife));
    ResourceLocation inTileName = Block::blockRegistry->getKey(mInTile);
    tag->putString(L"inTile", inTileName.toString());
    tag->putByte(L"inData", static_cast<unsigned char>(mInData));
    tag->putByte(L"shake", static_cast<unsigned char>(mShake));
    tag->putByte(L"inGround", static_cast<unsigned char>(mInGround));
    tag->putByte(L"pickup", static_cast<unsigned char>(mPickup));
    tag->putBoolean(L"crit", isCritArrow());
}

void AbstractRotatedItemProjectile::readAdditionalSaveData(CompoundTag* tag)
{
    mXTile = tag->getInt(L"xTile");
    mYTile = tag->getInt(L"yTile");
    mZTile = tag->getInt(L"zTile");
    mLife = tag->getShort(L"life");
    if (tag->contains(L"inTile", Tag::TAG_STRING))
        mInTile = Block::byString(tag->getString(L"inTile"));
    else
        mInTile = Block::byId(tag->getByte(L"inTile"));
    mInData = tag->getByte(L"inData");
    mShake = tag->getByte(L"shake");
    mInGround = tag->getByte(L"inGround") == 1;
    if (tag->contains(L"pickup", Tag::TAG_ANY_NUMERIC))
        mPickup = static_cast<ePickup>(tag->getByte(L"pickup"));
    else if (tag->contains(L"player", Tag::TAG_ANY_NUMERIC))
        mPickup = tag->getBoolean(L"player") ? ePickup_Allowed : ePickup_Disallowed;
    setCritArrow(tag->getBoolean(L"crit"));
}

void AbstractRotatedItemProjectile::playerTouch(boost::shared_ptr<Player> player)
{
    if (!mLevel->mIsClientSide && (mInGround || isNoPhysics()) && mShake < 1)
    {
        bool canPickup = mPickup == ePickup_Allowed || mPickup == ePickup_CreativeOnly
                         || (isNoPhysics() && mOwner.get() == player.get());
        if (mPickup == ePickup_Allowed)
        {
            if (!player->mInventory->add(getPickupItem()))
                spawnAtLocation(getPickupItem(), 1.0f);
        }
        if (canPickup)
        {
            player->take(shared_from_this(), 1);
            remove();
        }
    }
}

bool AbstractRotatedItemProjectile::isAttackable()
{
    return false;
}

float AbstractRotatedItemProjectile::getEyeHeight()
{
    return 0.0;
}

void AbstractRotatedItemProjectile::setNoPhysics(bool noPhysics)
{
    mNoPhysics = noPhysics;
    setFlag(2, noPhysics);
}

bool AbstractRotatedItemProjectile::shouldTick()
{
    return true;
}
