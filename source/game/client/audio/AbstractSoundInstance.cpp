#include "game/client/audio/AbstractSoundInstance.hpp"

AbstractSoundInstance::AbstractSoundInstance(const SoundEvent* soundEvent, SoundSource::ESoundSource soundSource)
{
    this->sound = soundEvent;
    this->looping = false;
    this->source = soundSource;
    this->attenuation = SoundInstance::DEFAULT_ATTENUATION;
    this->volume = 1.0f;
    this->delay = 0;
    this->pitch = 1.0f;
}

const SoundEvent* AbstractSoundInstance::getSound()
{
    return this->sound;
}

SoundSource::ESoundSource AbstractSoundInstance::getSource()
{
    return this->source;
}

bool AbstractSoundInstance::isLooping()
{
    return this->looping;
}

int AbstractSoundInstance::getDelay()
{
    return this->delay;
}

float AbstractSoundInstance::getVolume()
{
    return this->volume;
}

float AbstractSoundInstance::getPitch()
{
    return this->pitch;
}

float AbstractSoundInstance::getX()
{
    return this->x;
}

float AbstractSoundInstance::getY()
{
    return this->y;
}

float AbstractSoundInstance::getZ()
{
    return this->z;
}

SoundInstance::EAttenuation AbstractSoundInstance::getAttenuation()
{
    return this->attenuation;
}
