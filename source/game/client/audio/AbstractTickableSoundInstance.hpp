#ifndef GAME_CLIENT_AUDIO_ABSTRACTTICKABLESOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_ABSTRACTTICKABLESOUNDINSTANCE_HPP

#include "game/client/audio/AbstractSoundInstance.hpp"
#include "game/client/audio/TickableSoundInstance.hpp"

class AbstractTickableSoundInstance : public AbstractSoundInstance, public TickableSoundInstance
{
public:
    AbstractTickableSoundInstance(const SoundEvent* sound, SoundSource::ESoundSource source);
    AbstractTickableSoundInstance(const AbstractTickableSoundInstance& other);

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
    virtual ~AbstractTickableSoundInstance();
    virtual bool isStopped();
    virtual void stop();

protected:
    bool stopped;
};

#endif
