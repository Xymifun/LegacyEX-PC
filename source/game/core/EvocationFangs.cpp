#include "game/core/EvocationFangs.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/server/ServerLevel.hpp"

EvocationFangs::EvocationFangs(Level* level) : Entity(level, true), mOwner()
{
    _init();
    setSize(0.5f, 0.8f);
}

EvocationFangs::EvocationFangs(Level* level, double x, double y, double z, float yRot, int warmupDelay,
                               const boost::shared_ptr<LivingEntity>& owner)
    : Entity(level, true),
      mOwner()
{
    _init();
    mWarmupDelay = warmupDelay;
    setOwner(owner);
    mYRot = yRot * 57.295776f;
    setPos(x, y, z);
}

void EvocationFangs::_init()
{
    fjDerivedCtorCalls();
    mWarmupDelay = 0;
    mSentSpikeEvent = false;
    mLifeTicks = 22;
    mClientSideAttack = false;
    std::wstring ownerUUID(L"");
}

void EvocationFangs::setOwner(const boost::shared_ptr<LivingEntity>& owner)
{
    mOwner = owner;
    mOwnerUUID = owner == NULL ? std::wstring(L"") : owner->getUUID();
}

boost::shared_ptr<LivingEntity> EvocationFangs::getOwner()
{
    ServerLevel* serverLevel = dynamic_cast<ServerLevel*>(mLevel);
    if (mOwner.lock() == NULL && mOwnerUUID != L"" && serverLevel != NULL)
    {
        boost::shared_ptr<Entity> entity = serverLevel->getEntityByUUID(mOwnerUUID);
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
            mOwner = boost::static_pointer_cast<LivingEntity>(entity);
    }
    return mOwner.lock();
}

void EvocationFangs::defineSynchedData()
{
}

void EvocationFangs::readAdditionalSaveData(CompoundTag* tag)
{
    mWarmupDelay = tag->getInt(L"Warmup");
    mOwnerUUID = tag->getString(L"OwnerUUID");
}

void EvocationFangs::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Warmup", mWarmupDelay);
    if (mOwnerUUID != L"")
        tag->putString(L"OwnerUUID", mOwnerUUID);
}

void EvocationFangs::dealDamageTo(const boost::shared_ptr<LivingEntity>& target)
{
    boost::shared_ptr<LivingEntity> owner = getOwner();
    if (!target->isAlive() || target->isInvulnerable() || target == owner)
    {
        return;
    }

    if (owner == NULL)
    {
        target->hurt(DamageSource::MAGIC, 6.0f);
    }
    else
    {
        if (owner->isAlliedTo(target))
        {
            return;
        }

        DamageSource* damage = DamageSource::CreateIndirectMagic(shared_from_this(), owner, DamageSource::MAGIC);
        target->hurt(damage, 6.0f);
        delete damage;
    }
}

void EvocationFangs::tick()
{
    Entity::tick();

    if (mLevel->mIsClientSide)
    {
        if (mClientSideAttack)
        {
            --mLifeTicks;
            if (mLifeTicks == 14)
            {
                for (int i = 0; i < 12; ++i)
                {
                    double x = mX + (mRandom->nextDouble() * 2.0 - 1.0) * (double)mBbWidth * 0.5;
                    double y = mY + 0.05 + mRandom->nextDouble() * 1.0;
                    double z = mZ + (mRandom->nextDouble() * 2.0 - 1.0) * (double)mBbWidth * 0.5;
                    double xa = (mRandom->nextDouble() * 2.0 - 1.0) * 0.3;
                    double ya = 0.3 + mRandom->nextDouble() * 0.3;
                    double za = (mRandom->nextDouble() * 2.0 - 1.0) * 0.3;
                    arrayWithLength<int> parameters;
                    mLevel->addParticle(PARTICLE_CRIT, x, y + 1.0, z, xa, ya, za, parameters);
                }
            }
        }
    }
    else if (--mWarmupDelay < 0)
    {
        if (mWarmupDelay == -8)
        {
            Level* level = mLevel;
            std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(
                typeid(LivingEntity), mBoundingBox->grow(0.2, 0.0, 0.2));
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
            {
                dealDamageTo(boost::static_pointer_cast<LivingEntity>(*it));
            }
            delete entities;
        }
        if (!mSentSpikeEvent)
        {
            mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::START_ATTACKING, 0);
            mSentSpikeEvent = true;
        }
        if (--mLifeTicks < 0)
            remove();
    }
}

void EvocationFangs::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::START_ATTACKING)
    {
        mClientSideAttack = true;
        if (!isSilent())
        {
            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::EVOCATION_FANGS_ATTACK, getSoundSource(), 1.0f,
                                   0.85f + mRandom->nextFloat() * 0.2f, false, 16.0f);
        }
    }
    else
    {
        Entity::handleEntityEvent(event, data);
    }
}

float EvocationFangs::getAnimationProgress(float partialTicks) const
{
    if (!mClientSideAttack)
        return 0.0f;
    int ticks = mLifeTicks - 2;
    return ticks <= 0 ? 1.0f : 1.0f - ((float)ticks - partialTicks) / 20.0f;
}

Entity* EvocationFangs::create(Level* level)
{
    return new EvocationFangs(level);
}

unsigned long long EvocationFangs::GetType()
{
    return EntityType::EVOCATION_FANGS;
}
