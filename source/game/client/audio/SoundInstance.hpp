#ifndef GAME_CLIENT_AUDIO_SOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_SOUNDINSTANCE_HPP

#include "game/client/audio/SoundSource.hpp"

class SoundEvent;

class SoundInstance
{
public:
    enum eSoundState
    {
        eSoundState_None,
        eSoundState_Pending,
        eSoundState_Starting,
        eSoundState_Playing,
        eSoundState_Complete
    };

    enum EAttenuation
    {
        eAttenuation_Linear,
        eAttenuation_None
    };

    static const EAttenuation DEFAULT_ATTENUATION;

    SoundInstance() : drcClone(false), state(eSoundState_None), pendingTicks(0) {}

    virtual ~SoundInstance() {}

    virtual SoundInstance* getDRCClone() const = 0;
    virtual const SoundEvent* getSound() = 0;
    virtual SoundSource::ESoundSource getSource() = 0;
    virtual bool isLooping() = 0;
    virtual int getDelay() = 0;
    virtual float getVolume() = 0;
    virtual float getPitch() = 0;
    virtual float getX() = 0;
    virtual float getY() = 0;
    virtual float getZ() = 0;
    virtual EAttenuation getAttenuation() = 0;

    void setSIState(eSoundState state);
    bool isPending();
    bool isPlaying();
    bool isComplete();

    bool drcClone;
    eSoundState state;
    int pendingTicks;
};

#endif
