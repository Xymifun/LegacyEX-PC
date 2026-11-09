#ifndef GAME_CLIENT_AUDIO_SOUNDTYPE_HPP
#define GAME_CLIENT_AUDIO_SOUNDTYPE_HPP

#include "game/client/audio/SoundEvent.hpp"

class SoundType
{
public:
    static const SoundType* WOOD;
    static const SoundType* GRAVEL;
    static const SoundType* GRASS;
    static const SoundType* STONE;
    static const SoundType* METAL;
    static const SoundType* GLASS;
    static const SoundType* CLOTH;
    static const SoundType* SAND;
    static const SoundType* SNOW;
    static const SoundType* LADDER;
    static const SoundType* ANVIL;
    static const SoundType* SLIME;

    static void StaticCtor();

    SoundType(float volume, float pitch, const SoundEvent* breakSound, const SoundEvent* stepSound,
              const SoundEvent* placeSound, const SoundEvent* hitSound, const SoundEvent* fallSound);

    float getVolume() const;
    float getPitch() const;

    const SoundEvent* getBreakSound() const;
    const SoundEvent* getStepSound() const;
    const SoundEvent* getPlaceSound() const;
    const SoundEvent* getHitSound() const;
    const SoundEvent* getFallSound() const;

private:
    float volume;
    float pitch;
    const SoundEvent* breakSound;
    const SoundEvent* stepSound;
    const SoundEvent* placeSound;
    const SoundEvent* hitSound;
    const SoundEvent* fallSound;
};

#endif
