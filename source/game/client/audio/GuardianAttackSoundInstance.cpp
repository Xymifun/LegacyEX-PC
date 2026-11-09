#include "game/client/audio/GuardianAttackSoundInstance.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/Guardian.hpp"

static const float MAX_ATTACK_VOLUME = 1.0f;
static const float ATTACK_VOLUME_SCALE = MAX_ATTACK_VOLUME;

GuardianAttackSoundInstance::GuardianAttackSoundInstance(boost::shared_ptr<Guardian> guardian)
    : AbstractTickableSoundInstance(SoundEvents::GUARDIAN_ATTACK, SoundSource::eSoundSource_Hostile),
      guardian(guardian)
{
    this->attenuation = SoundInstance::DEFAULT_ATTENUATION;
    this->looping = true;
    this->delay = 0;
}

GuardianAttackSoundInstance::GuardianAttackSoundInstance(const GuardianAttackSoundInstance& other)
    : SoundInstance(other),
      AbstractTickableSoundInstance(other),
      guardian(other.guardian)
{
}

GuardianAttackSoundInstance::~GuardianAttackSoundInstance()
{
}

SoundInstance* GuardianAttackSoundInstance::getDRCClone() const
{
    GuardianAttackSoundInstance* clone = new GuardianAttackSoundInstance(*this);
    clone->drcClone = true;
    AbstractTickableSoundInstance* result = clone;
    return result;
}

void GuardianAttackSoundInstance::tick()
{
    if (this->guardian->mRemoved || !this->guardian->hasActiveAttackTarget())
    {
        this->stopped = true;
        return;
    }

    this->x = static_cast<float>(this->guardian->mX);
    this->y = static_cast<float>(this->guardian->mY);
    this->z = static_cast<float>(this->guardian->mZ);

    const float attackAnimationScale = this->guardian->getAttackAnimationScale(0.0f);
    this->volume = ATTACK_VOLUME_SCALE * attackAnimationScale * attackAnimationScale;
    this->pitch = 0.7f + 0.5f * attackAnimationScale;
}
