#ifndef GAME_CLIENT_AUDIO_ELYTRAONPLAYERSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_ELYTRAONPLAYERSOUNDINSTANCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/AbstractTickableSoundInstance.hpp"

class Player;

class ElytraOnPlayerSoundInstance : public AbstractTickableSoundInstance
{
public:
    ElytraOnPlayerSoundInstance(boost::shared_ptr<Player> player);
    ElytraOnPlayerSoundInstance(const ElytraOnPlayerSoundInstance& other);
    virtual ~ElytraOnPlayerSoundInstance();

    virtual void tick();
    virtual SoundInstance* getDRCClone() const;

private:
    int ticks;
    boost::shared_ptr<Player> player;
    int warningSoundDelay;
    bool warning;
};

#endif
