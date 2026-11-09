#ifndef GAME_CLIENT_AUDIO_TICKABLESOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_TICKABLESOUNDINSTANCE_HPP

#include "game/client/audio/SoundInstance.hpp"

class TickableSoundInstance : public virtual SoundInstance
{
public:
    TickableSoundInstance();
    TickableSoundInstance(const TickableSoundInstance& other);

    virtual bool isStopped() = 0;
    virtual void stop() = 0;
    virtual void tick() = 0;
    virtual ~TickableSoundInstance() {}
};

#endif
