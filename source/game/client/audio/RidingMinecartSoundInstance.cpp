#include "game/client/audio/RidingMinecartSoundInstance.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mth.hpp"
#include "game/world/generation/Player.hpp"

RidingMinecartSoundInstance::RidingMinecartSoundInstance(boost::shared_ptr<Player> player,
                                                         boost::shared_ptr<Minecart> minecart)
    : AbstractTickableSoundInstance(SoundEvents::MINECART_INSIDE, SoundSource::eSoundSource_Neutral)
{
    this->player = player;
    this->minecart = minecart;
    this->delay = 0;
    this->attenuation = SoundInstance::DEFAULT_ATTENUATION;
    this->looping = true;
}

RidingMinecartSoundInstance::RidingMinecartSoundInstance(const RidingMinecartSoundInstance& other)
    : SoundInstance(other),
      AbstractTickableSoundInstance(other),
      player(other.player),
      minecart(other.minecart)
{
}

RidingMinecartSoundInstance::~RidingMinecartSoundInstance()
{
}

SoundInstance* RidingMinecartSoundInstance::getDRCClone() const
{
    RidingMinecartSoundInstance* clone = new RidingMinecartSoundInstance(*this);
    clone->drcClone = true;
    AbstractTickableSoundInstance* result = clone;
    return result;
}

void RidingMinecartSoundInstance::tick()
{
    if (GameSettings::GetGameSettings(eGameSetting_MinecartSounds) != 0)
    {
        if (this->minecart->mRemoved || !this->player->isPassenger()
            || this->player->getVehicle().get() != this->minecart.get())
        {
            this->stopped = true;
            return;
        }

        this->x = static_cast<float>(this->minecart->mX);
        this->y = static_cast<float>(this->minecart->mY);
        this->z = static_cast<float>(this->minecart->mZ);

        const float speed = Mth::sqrt(this->minecart->mXd * this->minecart->mXd
                                      + this->minecart->mZd * this->minecart->mZd);
        if (speed >= 0.01)
        {
            this->volume = Mth::clamp(speed, 0.0f, 1.0f) * 0.75f;
            return;
        }
    }

    this->volume = 0.0f;
}
