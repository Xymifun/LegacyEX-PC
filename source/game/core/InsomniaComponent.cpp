#include "game/core/InsomniaComponent.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"

InsomniaComponent::InsomniaComponent(LivingEntity* entity, float daysUntilInsomnia)
    : mEntity(entity),
      mInsomniaTimerTicks(0),
      mDaysUntilInsomnia(daysUntilInsomnia),
      mTicksUntilInsomnia((int)(daysUntilInsomnia * 24000.0f))
{
}

void InsomniaComponent::restartTimer()
{
    mInsomniaTimerTicks = 0;
}

void InsomniaComponent::tick()
{
    if (mEntity->isSleeping())
    {
        restartTimer();
        return;
    }

    mInsomniaTimerTicks = mInsomniaTimerTicks + 1;
}

int InsomniaComponent::getInsomniaTimerTicks() const
{
    return mInsomniaTimerTicks;
}

int InsomniaComponent::getTicksUntilInsomnia() const
{
    return mTicksUntilInsomnia;
}

bool InsomniaComponent::hasInsomnia() const
{
    return mInsomniaTimerTicks > 0;
}

void InsomniaComponent::setInsomnia(int timerTicks)
{
    mInsomniaTimerTicks = timerTicks;
}

void InsomniaComponent::addAdditionalSaveData(CompoundTag* tag)
{
    tag->putInt(L"TimeSinceRest", mInsomniaTimerTicks);
}

void InsomniaComponent::readAdditionalSaveData(CompoundTag* tag)
{
    if (tag->contains(L"TimeSinceRest"))
    {
        mInsomniaTimerTicks = tag->getInt(L"TimeSinceRest");
    }
}
