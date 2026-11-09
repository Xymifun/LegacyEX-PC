#include "game/entity/projectile/ThrowableProjectile.hpp"
#include "game/core/EntityType.hpp"
#include <cmath>
#include <vector>
#include <boost/unordered_set.hpp>
#include "game/client/render/ParticleType.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/PIXNamedEvents.hpp"

ThrowableProjectile::ThrowableProjectile(Level* level)
    : Entity(level, true),
      Projectile(level, boost::shared_ptr<LivingEntity>())
{
    _throwableInit();
}

ThrowableProjectile::ThrowableProjectile(Level* level, boost::shared_ptr<LivingEntity> owner)
    : Entity(level, true),
      Projectile(level, owner)
{
    _throwableInit();
    setPos(owner->mX, owner->mY + owner->getEyeHeight(), owner->mZ);
}

ThrowableProjectile::ThrowableProjectile(Level* level, double x, double y, double z)
    : Entity(level, true),
      Projectile(level, boost::shared_ptr<LivingEntity>())
{
    _throwableInit();
    setPos(x, y, z);
}

ThrowableProjectile::~ThrowableProjectile()
{
    delete mParticleSpawnPos;
}

void ThrowableProjectile::shoot(double x, double y, double z, float velocity, float inaccuracy)
{
    double length = std::sqrt(x * x + y * y + z * z);
    x /= length;
    y /= length;
    z /= length;
    x += mRandom->nextGaussian() * 0.007499999832361937 * inaccuracy;
    y += mRandom->nextGaussian() * 0.007499999832361937 * inaccuracy;
    z += mRandom->nextGaussian() * 0.007499999832361937 * inaccuracy;
    x *= velocity;
    y *= velocity;
    z *= velocity;
    mXd = x;
    mYd = y;
    mZd = z;
    float horizontal = (float)std::sqrt(x * x + z * z);
    mYRotO = mYRot = (float)(std::atan2(x, z) * Mth::RAD_TO_DEG);
    mXRotO = mXRot = (float)(std::atan2(y, (double)horizontal) * Mth::RAD_TO_DEG);
    mLife = 0;
}

void ThrowableProjectile::shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset,
                                            float velocity, float inaccuracy)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        unsigned long long type = GetType();
        velocity = (float)miniGame->getThrowPower(type, velocity);
        offset = (float)miniGame->getThrowAngle(type, offset);
    }

    float x = -Mth::sin(yRot * Mth::DEG_TO_RAD) * Mth::cos(xRot * Mth::DEG_TO_RAD);
    float y = -Mth::sin((xRot + offset) * Mth::DEG_TO_RAD);
    float z = Mth::cos(yRot * Mth::DEG_TO_RAD) * Mth::cos(xRot * Mth::DEG_TO_RAD);
    shoot(x, y, z, velocity, inaccuracy);
    mXd += entity->mXd;
    mZd += entity->mZd;
    if (!entity->mOnGround)
        mYd += entity->mYd;
}

void ThrowableProjectile::fjSetParticleSpawnPos(double x, double y, double z)
{
    if (mParticleSpawnPos == NULL)
        mParticleSpawnPos = Vec3::newPermanent(x, y, z);
}

void ThrowableProjectile::addDataWalkers(DataFixerUpper*, unsigned long long)
{
}

void ThrowableProjectile::_throwableInit()
{
    mXTile = -1;
    mYTile = -1;
    mZTile = -1;
    mInTile = Blocks::air;
    mInGround = false;
    mShake = 0;
    mLife = 0;
    mFlightTime = 0;
    mOwnerName = L"";
    setSize(0.25f, 0.25f);
    mParticleSpawnPos = NULL;
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureThrownProjectilesAreFireImmune())
        mFireImmune = true;
}

void ThrowableProjectile::defineSynchedData()
{
}

bool ThrowableProjectile::shouldRenderAtSqrDistance(double distanceSqr)
{
    double distance = mBoundingBox->getSize() * 4.0;
    if (Mth::isNaN(distance))
        distance = 4.0;
    distance *= 64.0;
    return distanceSqr < distance * distance;
}

void ThrowableProjectile::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRotO == 0.0f && mYRotO == 0.0f)
    {
        double horizontal = std::sqrt(xd * xd + zd * zd);
        mYRot = (float)(std::atan2(xd, zd) * Mth::RAD_TO_DEG);
        mYRotO = mYRot;
        mXRot = (float)(std::atan2(yd, horizontal) * Mth::RAD_TO_DEG);
        mXRotO = mXRot;
    }
}

void ThrowableProjectile::tick()
{
    PIXBeginNamedEvent(0.0f, "Throwable projectile tick");
    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    Entity::tick();
    if (mShake > 0)
        --mShake;

    HitResult* hitResult = NULL;
    if (mInGround)
    {
        Block* block = mLevel->getBlockState(BlockPos(mXTile, mYTile, mZTile))->getBlock();
        if (block == mInTile)
        {
            ++mLife;
            if (mLife == 1200)
                remove();
            PIXEndNamedEvent();
            return;
        }

        mInGround = false;
        mXd *= mRandom->nextFloat() * 0.2f;
        mYd *= mRandom->nextFloat() * 0.2f;
        mZd *= mRandom->nextFloat() * 0.2f;
        mLife = 0;
        mFlightTime = 0;
    }
    else
    {
        ++mFlightTime;
    }
    Vec3* from = Vec3::newTemp(mX, mY, mZ);
    Vec3* to = Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd);

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        typedef boost::unordered_set<Block*, boost::hash<Block*>, std::equal_to<Block*>, std::allocator<Block*> >
            BlockSet;
        BlockSet* alwaysClipBlocks = NULL;
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        {
            alwaysClipBlocks = new BlockSet();
            alwaysClipBlocks->insert(Blocks::web);
            alwaysClipBlocks->insert(Blocks::vine);
        }
        hitResult = mLevel->clip(from, to, false, true, false, false, alwaysClipBlocks, false);
        delete alwaysClipBlocks;
    }
    else
    {
        hitResult = mLevel->clip(from, to);
    }

    from = Vec3::newTemp(mX, mY, mZ);
    to = Vec3::newTemp(mX + mXd, mY + mYd, mZ + mZd);
    if (hitResult != NULL)
        to = Vec3::newTemp(hitResult->mLocation->x, hitResult->mLocation->y, hitResult->mLocation->z);

    if (!mLevel->mIsClientSide && !Minecraft::GetInstance()->GetMiniGame()->HasFeatureGhostThroughProjectiles())
    {
        boost::shared_ptr<Entity> hitEntity;
        double nearestDistance = 0.0;
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntities(
            shared_from_this(), mBoundingBox->expand(mXd, mYd, mZd)->grow(1.0, 1.0, 1.0), NULL);
        boost::shared_ptr<LivingEntity> owner = getOwner();
        bool shorterHitbox = Minecraft::GetInstance()->GetMiniGame()->HasFeatureShorterHitbox();

        for (unsigned int i = 0; i < entities->size(); ++i)
        {
            boost::shared_ptr<Entity> entity = entities->at(i);
            if (!entity->isPickable())
                continue;
            if (owner && (entity == owner || entity == owner->getVehicle()) && mFlightTime < 5)
                continue;

            AABB* hitbox = shorterHitbox ? entity->mBoundingBox->scaleAndShift(1.0, 0.8, 1.0)
                                         : entity->mBoundingBox->grow(0.30000001192092896);
            HitResult* entityHit = hitbox->clip(from, to);
            if (entityHit != NULL)
            {
                double distance = from->distanceToSqr(entityHit->mLocation);
                delete entityHit;
                if (distance < nearestDistance || nearestDistance == 0.0)
                {
                    hitEntity = entity;
                    nearestDistance = distance;
                }
            }
        }

        if (hitEntity)
        {
            delete hitResult;
            hitResult = new HitResult(hitEntity);
        }
    }

    if (hitResult != NULL)
    {
        if (hitResult->mType == HitResult::BLOCK
            && mLevel->getBlockState(hitResult->getBlockPos())->getBlock() == Blocks::portal)
            handleInsidePortal(hitResult->getBlockPos());
        else
            onHit(hitResult);
        delete hitResult;
    }

    mX += mXd;
    mY += mYd;
    mZ += mZd;
    double horizontal = std::sqrt(mXd * mXd + mZd * mZd);
    mYRot = (float)(std::atan2(mXd, mZd) * 57.295776f);
    mXRot = (float)(std::atan2(mYd, horizontal) * 57.295776f);

    while (mXRot - mXRotO < -180.0f)
        mXRotO -= 360.0f;
    while (mXRot - mXRotO >= 180.0f)
        mXRotO += 360.0f;
    while (mYRot - mYRotO < -180.0f)
        mYRotO -= 360.0f;
    while (mYRot - mYRotO >= 180.0f)
        mYRotO += 360.0f;

    mXRot = mXRotO + (mXRot - mXRotO) * 0.2f;
    mYRot = mYRotO + (mYRot - mYRotO) * 0.2f;
    float gravity = getGravity();
    float inertia = 0.99f;
    if (isInWater())
    {
        for (int i = 0; i < 4; ++i)
            mLevel->addParticle(PARTICLE_BUBBLE, mX - mXd * 0.25, mY - mYd * 0.25, mZ - mZd * 0.25, mXd, mYd, mZd,
                                arrayWithLength<int>());
        inertia = 0.8f;
    }
    mXd *= inertia;
    mYd *= inertia;
    mZd *= inertia;
    if (!isNoGravity())
        mYd -= gravity;
    setPos(mX, mY, mZ);
    PIXEndNamedEvent();
}

float ThrowableProjectile::getGravity()
{
    float gravity = 0.03f;
    return gravity;
}

void ThrowableProjectile::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"xTile", mXTile);
    tag->putInt(L"yTile", mYTile);
    tag->putInt(L"zTile", mZTile);
    ResourceLocation inTileName = Block::blockRegistry->getKey(mInTile);
    tag->putString(L"inTile", inTileName.toString());
    tag->putByte(L"shake", (unsigned char)mShake);
    tag->putByte(L"inGround", (unsigned char)mInGround);
    if (mOwnerName.empty() && mOwner)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, mOwner->GetType()))
            mOwnerName = mOwner->getName();
    }
    tag->putString(L"ownerName", mOwnerName.empty() ? L"" : mOwnerName);
}

void ThrowableProjectile::readAdditionalSaveData(CompoundTag* tag)
{
    mXTile = tag->getInt(L"xTile");
    mYTile = tag->getInt(L"yTile");
    mZTile = tag->getInt(L"zTile");
    if (tag->contains(L"inTile", Tag::TAG_STRING))
        mInTile = Block::byString(tag->getString(L"inTile"));
    else
        mInTile = Block::byId(tag->getByte(L"inTile"));
    mShake = tag->getByte(L"shake");
    mInGround = tag->getByte(L"inGround") == 1;
    mOwner = boost::shared_ptr<LivingEntity>();
    mOwnerName = tag->getString(L"ownerName");
    if (mOwnerName.empty())
        mOwnerName = L"";
    mOwner = getOwner();
}

boost::shared_ptr<LivingEntity> ThrowableProjectile::getOwner()
{
    if (!mOwner && !mOwnerName.empty())
        mOwner = mLevel->getPlayerByName(mOwnerName);
    return mOwner;
}

Vec3* ThrowableProjectile::fjGetParticleSpawnPos() const
{
    if (mParticleSpawnPos != NULL)
        return mParticleSpawnPos;
    return Vec3::newTemp(mX, mY, mZ);
}

bool ThrowableProjectile::shouldTick()
{
    return true;
}
