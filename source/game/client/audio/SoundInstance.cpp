#include "game/client/audio/SoundInstance.hpp"

const SoundInstance::EAttenuation SoundInstance::DEFAULT_ATTENUATION = eAttenuation_Linear;

void SoundInstance::setSIState(eSoundState state)
{
    this->state = state;
}

bool SoundInstance::isPending()
{
    return state == eSoundState_Pending;
}

bool SoundInstance::isComplete()
{
    return state == eSoundState_Complete;
}

bool SoundInstance::isPlaying()
{
    return state == eSoundState_Playing;
}
