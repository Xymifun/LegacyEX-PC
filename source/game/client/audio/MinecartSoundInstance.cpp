#include "game/client/audio/MinecartSoundInstance.hpp"

#include <cmath>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mth.hpp"

MinecartSoundInstance::MinecartSoundInstance(boost::shared_ptr<Minecart> minecart)
    : AbstractTickableSoundInstance(SoundEvents::MINECART_RIDING, SoundSource::eSoundSource_Neutral),
      minecart(minecart),
      distance(0.0f)
{
    this->looping = true;
    this->delay = 0;
    this->tick();
}

MinecartSoundInstance::MinecartSoundInstance(const MinecartSoundInstance& other)
    : SoundInstance(other),
      AbstractTickableSoundInstance(other),
      minecart(other.minecart),
      distance(other.distance)
{
}

MinecartSoundInstance::~MinecartSoundInstance()
{
}

SoundInstance* MinecartSoundInstance::getDRCClone() const
{
    MinecartSoundInstance* clone = new MinecartSoundInstance(*this);
    clone->drcClone = true;
    AbstractTickableSoundInstance* result = clone;
    return result;
}

void MinecartSoundInstance::tick()
{
    if (GameSettings::GetGameSettings(eGameSetting_MinecartSounds) == 0)
    {
        this->volume = 0.0f;
        return;
    }

    if (this->minecart->mRemoved)
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->minecart->mX);
    this->y = static_cast<float>(this->minecart->mY);
    this->z = static_cast<float>(this->minecart->mZ);

    const double speed = std::sqrt(this->minecart->mXd * this->minecart->mXd
                                   + this->minecart->mZd * this->minecart->mZd);
    if (speed >= 0.01)
    {
        this->distance = Mth::clamp(this->distance + 0.0025f, 0.0f, 1.0f);
        this->volume = static_cast<float>(Mth::clamp(speed, 0.0, 0.5) * 0.699999988079071);
    }
    else
    {
        this->distance = 0.0f;
        this->volume = 0.0f;
    }
}
