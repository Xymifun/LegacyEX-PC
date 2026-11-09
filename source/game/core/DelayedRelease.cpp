#include "game/core/DelayedRelease.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/core/DamageSource.hpp"
#include "game/world/Level.hpp"

DelayedRelease::DelayedRelease(Level* level, boost::shared_ptr<Entity> entity, int delay) : Entity(level, true)
{
    moveTo(entity->mX, entity->mY, entity->mZ, 0.0f, 0.0f);
    mEntity = entity;
    mDelay = delay;
}

bool DelayedRelease::makeStepSound()
{
    return false;
}

void DelayedRelease::tick()
{
    if (mDelay-- < 1)
    {
        mLevel->addEntity(mEntity);
        remove();
    }
}

bool DelayedRelease::hurt(const DamageSource*, float)
{
    return false;
}

void DelayedRelease::defineSynchedData()
{
}

void DelayedRelease::readAdditionalSaveData(CompoundTag*)
{
}

void DelayedRelease::addAdditonalSaveData(CompoundTag*)
{
}

unsigned long long DelayedRelease::GetType()
{
    return EntityType::DELAYED_RELEASE;
}
