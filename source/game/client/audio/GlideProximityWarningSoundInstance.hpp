#ifndef GAME_CLIENT_AUDIO_GLIDEPROXIMITYWARNINGSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_GLIDEPROXIMITYWARNINGSOUNDINSTANCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/AbstractTickableSoundInstance.hpp"

class LocalPlayer;

class GlideProximityWarningSoundInstance : public AbstractTickableSoundInstance
{
public:
    GlideProximityWarningSoundInstance(boost::shared_ptr<LocalPlayer> player, const SoundEvent* sound);
    GlideProximityWarningSoundInstance(const GlideProximityWarningSoundInstance& other);
    virtual ~GlideProximityWarningSoundInstance();

    virtual void tick();
    virtual SoundInstance* getDRCClone() const;

private:
    boost::shared_ptr<LocalPlayer> player;
    unsigned int ticks;
};

#endif
