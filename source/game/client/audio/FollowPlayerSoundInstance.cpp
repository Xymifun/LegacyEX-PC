#include "game/client/audio/FollowPlayerSoundInstance.hpp"

#include "game/world/generation/Player.hpp"

FollowPlayerSoundInstance::FollowPlayerSoundInstance(boost::shared_ptr<Player> player, const SoundEvent* sound,
                                                     float volume, float pitch, bool looping)
    : AbstractTickableSoundInstance(sound, SoundSource::eSoundSource_Players),
      player()
{
    this->player = player;
    this->looping = looping;
    this->delay = 0;
    this->volume = volume;
    this->pitch = pitch;
}

FollowPlayerSoundInstance::FollowPlayerSoundInstance(const FollowPlayerSoundInstance& other)
    : SoundInstance(other),
      AbstractTickableSoundInstance(other),
      player(other.player)
{
}

FollowPlayerSoundInstance::~FollowPlayerSoundInstance()
{
}

SoundInstance* FollowPlayerSoundInstance::getDRCClone() const
{
    FollowPlayerSoundInstance* clone = new FollowPlayerSoundInstance(*this);
    clone->drcClone = true;
    AbstractTickableSoundInstance* result = clone;
    return result;
}

void FollowPlayerSoundInstance::tick()
{
    if (this->player->mRemoved)
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->player->mX);
    this->y = static_cast<float>(this->player->mY);
    this->z = static_cast<float>(this->player->mZ);
}
