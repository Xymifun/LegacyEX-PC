#ifndef GAME_CLIENT_AUDIO_GUARDIANATTACKSOUNDINSTANCE_HPP
#define GAME_CLIENT_AUDIO_GUARDIANATTACKSOUNDINSTANCE_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/AbstractTickableSoundInstance.hpp"

class Guardian;

class GuardianAttackSoundInstance : public AbstractTickableSoundInstance
{
public:
    GuardianAttackSoundInstance(boost::shared_ptr<Guardian> guardian);
    GuardianAttackSoundInstance(const GuardianAttackSoundInstance& other);
    virtual ~GuardianAttackSoundInstance();

    virtual void tick();
    virtual SoundInstance* getDRCClone() const;

private:
    boost::shared_ptr<Guardian> guardian;
};

#endif
