#include "game/core/Golem.hpp"

Golem::Golem(Level* level) : PathfinderMob(level)
{
}

void Golem::causeFallDamage(float, float)
{
}

const SoundEvent* Golem::getAmbientSound()
{
    return NULL;
}

const SoundEvent* Golem::getHurtSound(const DamageSource*)
{
    return NULL;
}

const SoundEvent* Golem::getDeathSound()
{
    return NULL;
}

int Golem::getAmbientSoundInterval()
{
    return 120;
}

bool Golem::removeWhenFarAway()
{
    return false;
}
