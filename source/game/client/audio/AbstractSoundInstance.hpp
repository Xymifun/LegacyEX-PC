#ifndef GAME_CLIENT_AUDIO_ABSTRACTSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_ABSTRACTSOUNDINSTANCE_HPP

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundInstance.hpp"
#include "game/client/audio/SoundSource.hpp"

class AbstractSoundInstance : public virtual SoundInstance
{
public:
    AbstractSoundInstance(const SoundEvent* soundEvent, SoundSource::ESoundSource soundSource);

    virtual const SoundEvent* getSound();
    virtual SoundSource::ESoundSource getSource();
    virtual bool isLooping();
    virtual int getDelay();
    virtual float getVolume();
    virtual float getPitch();
    virtual float getX();
    virtual float getY();
    virtual float getZ();
    virtual SoundInstance::EAttenuation getAttenuation();
    virtual ~AbstractSoundInstance() {}

protected:
    SoundSource::ESoundSource source;
    const SoundEvent* sound;
    float volume;
    float pitch;
    float x;
    float y;
    float z;
    bool looping;
    int delay;
    SoundInstance::EAttenuation attenuation;
};

#endif
