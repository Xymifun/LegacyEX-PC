#include "game/client/audio/SoundEvent.hpp"

SoundEvent::SoundEvent(std::wstring name, EType type, float distanceScalar) : index(-1)
{
    this->name = name;
    this->type = type;
    this->distanceScalar = distanceScalar;
}

SoundEvent::SoundEvent(std::wstring name, float distanceScalar) : index(-1), type(eType_Sound)
{
    this->name = name;
    this->distanceScalar = distanceScalar;
}

std::wstring SoundEvent::getName() const
{
    return this->name;
}

void SoundEvent::setIndex(int index)
{
    this->index = index;
}

float SoundEvent::getDistanceScalar() const
{
    return this->distanceScalar;
}

bool SoundEvent::isSFX() const
{
    return this->type == eType_SFX;
}

bool SoundEvent::isMusic() const
{
    return this->type == eType_Music;
}

int SoundEvent::getIndex() const
{
    return this->index;
}
