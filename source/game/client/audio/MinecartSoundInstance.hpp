#ifndef GAME_CLIENT_AUDIO_MINECARTSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_MINECARTSOUNDINSTANCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/AbstractTickableSoundInstance.hpp"

class Minecart;

class MinecartSoundInstance : public AbstractTickableSoundInstance
{
public:
    MinecartSoundInstance(boost::shared_ptr<Minecart> minecart);
    MinecartSoundInstance(const MinecartSoundInstance& other);
    virtual ~MinecartSoundInstance();

    virtual void tick();
    virtual SoundInstance* getDRCClone() const;

private:
    boost::shared_ptr<Minecart> minecart;
    float distance;
};

#endif
