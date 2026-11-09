#include "game/client/audio/AbstractTickableSoundInstance.hpp"

AbstractTickableSoundInstance::AbstractTickableSoundInstance(const SoundEvent* sound, SoundSource::ESoundSource source)
    : AbstractSoundInstance(sound, source),
      TickableSoundInstance(),
      stopped(false)
{
}

AbstractTickableSoundInstance::AbstractTickableSoundInstance(const AbstractTickableSoundInstance& other)
    : SoundInstance(other),
      AbstractSoundInstance(other),
      TickableSoundInstance(other),
      stopped(other.stopped)
{
}

AbstractTickableSoundInstance::~AbstractTickableSoundInstance()
{
}

const SoundEvent* AbstractTickableSoundInstance::getSound()
{
    return AbstractSoundInstance::getSound();
}

SoundSource::ESoundSource AbstractTickableSoundInstance::getSource()
{
    return this->source;
}

bool AbstractTickableSoundInstance::isLooping()
{
    return AbstractSoundInstance::isLooping();
}

int AbstractTickableSoundInstance::getDelay()
{
    return AbstractSoundInstance::getDelay();
}

float AbstractTickableSoundInstance::getVolume()
{
    return AbstractSoundInstance::getVolume();
}

float AbstractTickableSoundInstance::getPitch()
{
    return AbstractSoundInstance::getPitch();
}

float AbstractTickableSoundInstance::getX()
{
    return AbstractSoundInstance::getX();
}

float AbstractTickableSoundInstance::getY()
{
    return AbstractSoundInstance::getY();
}

float AbstractTickableSoundInstance::getZ()
{
    return AbstractSoundInstance::getZ();
}

SoundInstance::EAttenuation AbstractTickableSoundInstance::getAttenuation()
{
    return AbstractSoundInstance::getAttenuation();
}

bool AbstractTickableSoundInstance::isStopped()
{
    return this->stopped;
}

void AbstractTickableSoundInstance::stop()
{
    this->stopped = true;
}
