#ifndef GAME_CLIENT_AUDIO_FOLLOWPLAYERSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_FOLLOWPLAYERSOUNDINSTANCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/AbstractTickableSoundInstance.hpp"

class Player;

class FollowPlayerSoundInstance : public AbstractTickableSoundInstance
{
public:
    FollowPlayerSoundInstance(boost::shared_ptr<Player> player, const SoundEvent* sound, float volume, float pitch,
                              bool looping);
    FollowPlayerSoundInstance(const FollowPlayerSoundInstance& other);
    virtual ~FollowPlayerSoundInstance();

    virtual void tick();
    virtual SoundInstance* getDRCClone() const;

private:
    boost::shared_ptr<Player> player;
};

#endif
