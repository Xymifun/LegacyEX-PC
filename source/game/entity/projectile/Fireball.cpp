#include "game/entity/projectile/Fireball.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/Dimension.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <string>
#include <vector>

#include "game/core/HitResult.hpp"
#include "game/core/Mth.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/projectile/ProjectileUtil.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"

Fireball::Fireball(Level* level) : Entity(level, true), mOwner()
{
    _init();
    setSize(1.0f, 1.0f);
}

Fireball::Fireball(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Entity(level, true),
      mOwner()
{
    _init();
    setSize(1.0f, 1.0f);
    moveTo(x, y, z, mYRot, mXRot);
    setPos(x, y, z);
    const double length = std::sqrt(xd * xd + yd * yd + zd * zd);
    if (length == 0.0)
    {
        mPowerX = 0.0;
        mPowerY = 0.0;
        mPowerZ = 0.0;
    }
    else
    {
        mPowerX = xd / length * 0.1;
        mPowerY = yd / length * 0.1;
        mPowerZ = zd / length * 0.1;
    }
}

Fireball::Fireball(Level* level, boost::shared_ptr<LivingEntity> owner, double xd, double yd, double zd)
    : Entity(level, true),
      mOwner()
{
    _init();
    mOwner = owner;
    setSize(1.0f, 1.0f);
    moveTo(owner->mX, owner->mY, owner->mZ, owner->mYRot, owner->mXRot);
    setPos(mX, mY, mZ);
    mXd = 0.0;
    mYd = 0.0;
    mZd = 0.0;
    const double spread = 0.4;
    xd += mRandom->nextGaussian() * spread;
    yd += mRandom->nextGaussian() * spread;
    zd += mRandom->nextGaussian() * spread;
    const double length = std::sqrt(xd * xd + yd * yd + zd * zd);
    if (length == 0.0)
    {
        mPowerX = 0.0;
        mPowerY = 0.0;
        mPowerZ = 0.0;
    }
    else
    {
        mPowerX = xd / length * 0.1;
        mPowerY = yd / length * 0.1;
        mPowerZ = zd / length * 0.1;
    }
}

bool Fireball::shouldTick()
{
    return true;
}

void Fireball::addDataWalkers(DataFixerUpper*, unsigned long long)
{
}

void Fireball::_init()
{
    mUnknown = 0;
    mLife = 0;
    mOwner = boost::shared_ptr<LivingEntity>();
    mPowerX = 0.0;
    mPowerY = 0.0;
    mPowerZ = 0.0;
}

void Fireball::defineSynchedData()
{
}

bool Fireball::shouldRenderAtSqrDistance(double distanceSqr)
{
    double distance = mBoundingBox->getSize() * 4.0;
    if (Mth::isNaN(distance))
        distance = 4.0;
    distance *= 64.0;
    return distanceSqr < distance * distance;
}

void Fireball::tick()
{
    PIXBeginNamedEvent(0.0f, "Fireball tick");

    if (!mLevel->mIsClientSide)
    {
        if ((mOwner != NULL && mOwner->mRemoved) || !mLevel->hasChunkAt(BlockPos(this)))
        {
            remove();
            PIXEndNamedEvent();
            return;
        }
        else
        {
            ChunkPos chunkPos(BlockPos(this));
            if (!mLevel->mDimension->containsChunk(chunkPos.x, chunkPos.z))
            {
                remove();
                PIXEndNamedEvent();
                return;
            }
        }
    }

    Entity::tick();

    if (shouldBurn())
    {
        setSecondsOnFire(1);
    }

    std::vector<boost::shared_ptr<Entity> >* ignoredEntities = NULL;
    if (mOwner != NULL)
    {
        ignoredEntities = new std::vector<boost::shared_ptr<Entity> >();
        ignoredEntities->push_back(mOwner);
    }

    std::vector<unsigned long long>* ignoredTypes = getEntityTypesToIgnoreCollisionsWith();
    HitResult* hit = ProjectileUtil::forwardsRaycast(shared_from_this(), true, ignoredEntities, ignoredTypes);
    delete ignoredTypes;
    delete ignoredEntities;

    if (hit != NULL)
    {
        onHit(hit);
        delete hit;
    }

    mX += mXd;
    mY += mYd;
    mZ += mZd;
    ProjectileUtil::rotateTowardsMovement(shared_from_this(), 0.2f);

    float inertia = getInertia();

    if (isInWater())
    {
        for (int i = 0; i < 4; ++i)
        {
            mLevel->addParticle(PARTICLE_BUBBLE, mX - mXd * 0.25, mY - mYd * 0.25, mZ - mZd * 0.25, mXd, mYd, mZd,
                                arrayWithLength<int>());
        }

        inertia = 0.8f;
    }

    mXd += mPowerX;
    mYd += mPowerY;
    mZd += mPowerZ;
    mXd *= inertia;
    mYd *= inertia;
    mZd *= inertia;

    if (!mLevel->mIsClientSide && fabs(mXd) < 0.002 && fabs(mYd) < 0.002 && fabs(mZd) < 0.002)
    {
        mXd = 0.0;
        mYd = 0.0;
        mZd = 0.0;
        remove();
    }

    mLevel->addParticle(getTrailParticleType(), mX, mY + 0.5, mZ, 0.0, 0.01, 0.0, arrayWithLength<int>());
    setPos(mX, mY, mZ);

    PIXEndNamedEvent();
}

float Fireball::getInertia()
{
    return 0.95f;
}

void Fireball::addAdditonalSaveData(CompoundTag* tag)
{
    tag->put(L"direction", newDoubleList(3, mXd, mYd, mZd));
    tag->put(L"power", newDoubleList(3, mPowerX, mPowerY, mPowerZ));
    tag->putInt(L"life", mLife);
}

void Fireball::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"power", Tag::TAG_LIST))
    {
        ListTag* power = tag->getList(L"power", Tag::TAG_DOUBLE);
        if (power->size() == 3)
        {
            mPowerX = power->getDouble(0);
            mPowerY = power->getDouble(1);
            mPowerZ = power->getDouble(2);
        }
    }
    mLife = tag->getInt(L"life");
    if (tag->contains(L"direction", Tag::TAG_LIST))
    {
        ListTag* direction = tag->getList(L"direction", Tag::TAG_DOUBLE);
        mXd = direction->getDouble(0);
        mYd = direction->getDouble(1);
        mZd = direction->getDouble(2);
    }
    else
    {
        remove();
    }
}

bool Fireball::isPickable()
{
    return true;
}

float Fireball::getPickRadius()
{
    return 1.0f;
}

bool Fireball::hurt(const DamageSource* source, float)
{
    if (!isInvulnerableTo(source))
    {
        markHurt();
        if (source->getEntity() != NULL)
        {
            Vec3* direction = source->getEntity()->getLookAngle();
            if (direction != NULL)
            {
                mXd = direction->x;
                mYd = direction->y;
                mZd = direction->z;
                mPowerX = direction->x * 0.1;
                mPowerY = direction->y * 0.1;
                mPowerZ = direction->z * 0.1;
            }
            bool living = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, source->getEntity()->GetType());
            if (living)
            {
                mOwner = boost::dynamic_pointer_cast<LivingEntity>(source->getEntity());
            }
            return true;
        }
    }
    return false;
}

float Fireball::getBrightness()
{
    return static_cast<float>(1.0);
}

int Fireball::getLightColor()
{
    return 0xF000F0;
}

const ParticleType* Fireball::getTrailParticleType()
{
    return PARTICLE_SMOKE;
}

bool Fireball::shouldBurn()
{
    return true;
}

std::vector<unsigned long long>* Fireball::getEntityTypesToIgnoreCollisionsWith()
{
    return 0;
}

unsigned long long Fireball::GetType()
{
    return EntityType::FIREBALL;
}
