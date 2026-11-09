#include "game/client/audio/GlideProximityWarningSoundInstance.hpp"

#include "game/world/generation/LocalPlayer.hpp"

GlideProximityWarningSoundInstance::GlideProximityWarningSoundInstance(boost::shared_ptr<LocalPlayer> player,
                                                                       const SoundEvent* sound)
    : AbstractTickableSoundInstance(sound, SoundSource::eSoundSource_Players)
{
    this->player = player;
    this->volume = 1.0f;
    this->ticks = 0;
    this->looping = true;
    this->delay = 0;
}

GlideProximityWarningSoundInstance::GlideProximityWarningSoundInstance(const GlideProximityWarningSoundInstance& other)
    : SoundInstance(other),
      AbstractTickableSoundInstance(other),
      player(other.player),
      ticks(other.ticks)
{
}

GlideProximityWarningSoundInstance::~GlideProximityWarningSoundInstance()
{
}

SoundInstance* GlideProximityWarningSoundInstance::getDRCClone() const
{
    GlideProximityWarningSoundInstance* clone = new GlideProximityWarningSoundInstance(*this);
    clone->drcClone = true;
    AbstractTickableSoundInstance* result = clone;
    return result;
}

void GlideProximityWarningSoundInstance::tick()
{
    ++this->ticks;

    if (this->player->mRemoved)
    {
        this->stopped = true;
        return;
    }

    if (this->ticks > 20 && !this->player->isFallFlying())
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->player->mX);
    this->y = static_cast<float>(this->player->mY);
    this->z = static_cast<float>(this->player->mZ);

    if (this->player->getElytraProximityWarning() && this->player->getElytraCollisionDistance() < 2.0)
        this->volume = 0.8f;
}
