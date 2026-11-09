#include "game/client/audio/SoundType.hpp"

#include "game/client/audio/SoundEvents.hpp"

const SoundType* SoundType::WOOD = NULL;
const SoundType* SoundType::GRAVEL = NULL;
const SoundType* SoundType::GRASS = NULL;
const SoundType* SoundType::STONE = NULL;
const SoundType* SoundType::METAL = NULL;
const SoundType* SoundType::GLASS = NULL;
const SoundType* SoundType::CLOTH = NULL;
const SoundType* SoundType::SAND = NULL;
const SoundType* SoundType::SNOW = NULL;
const SoundType* SoundType::LADDER = NULL;
const SoundType* SoundType::ANVIL = NULL;
const SoundType* SoundType::SLIME = NULL;

SoundType::SoundType(float volume, float pitch, const SoundEvent* breakSound, const SoundEvent* stepSound,
                     const SoundEvent* placeSound, const SoundEvent* hitSound, const SoundEvent* fallSound)
    : volume(volume),
      pitch(pitch),
      breakSound(breakSound),
      stepSound(stepSound),
      placeSound(placeSound),
      hitSound(hitSound),
      fallSound(fallSound)
{
}

void SoundType::StaticCtor()
{
    WOOD = new SoundType(1.0f, 1.0f, SoundEvents::WOOD_BREAK, SoundEvents::WOOD_STEP, SoundEvents::WOOD_PLACE,
                         SoundEvents::WOOD_HIT, SoundEvents::WOOD_FALL);
    GRAVEL = new SoundType(1.0f, 1.0f, SoundEvents::GRAVEL_BREAK, SoundEvents::GRAVEL_STEP, SoundEvents::GRAVEL_PLACE,
                           SoundEvents::GRAVEL_HIT, SoundEvents::GRAVEL_FALL);
    GRASS = new SoundType(1.0f, 1.0f, SoundEvents::GRASS_BREAK, SoundEvents::GRASS_STEP, SoundEvents::GRASS_PLACE,
                          SoundEvents::GRASS_HIT, SoundEvents::GRASS_FALL);
    STONE = new SoundType(1.0f, 1.0f, SoundEvents::STONE_BREAK, SoundEvents::STONE_STEP, SoundEvents::STONE_PLACE,
                          SoundEvents::STONE_HIT, SoundEvents::STONE_FALL);
    METAL = new SoundType(1.0f, 1.5f, SoundEvents::METAL_BREAK, SoundEvents::METAL_STEP, SoundEvents::METAL_PLACE,
                          SoundEvents::METAL_HIT, SoundEvents::METAL_FALL);
    GLASS = new SoundType(1.0f, 1.0f, SoundEvents::GLASS_BREAK, SoundEvents::GLASS_STEP, SoundEvents::GLASS_PLACE,
                          SoundEvents::GLASS_HIT, SoundEvents::GLASS_FALL);
    CLOTH = new SoundType(1.0f, 1.0f, SoundEvents::CLOTH_BREAK, SoundEvents::CLOTH_STEP, SoundEvents::CLOTH_PLACE,
                          SoundEvents::CLOTH_HIT, SoundEvents::CLOTH_FALL);
    SAND = new SoundType(1.0f, 1.0f, SoundEvents::SAND_BREAK, SoundEvents::SAND_STEP, SoundEvents::SAND_PLACE,
                         SoundEvents::SAND_HIT, SoundEvents::SAND_FALL);
    SNOW = new SoundType(1.0f, 1.0f, SoundEvents::SNOW_BREAK, SoundEvents::SNOW_STEP, SoundEvents::SNOW_PLACE,
                         SoundEvents::SNOW_HIT, SoundEvents::SNOW_FALL);
    LADDER = new SoundType(1.0f, 1.0f, SoundEvents::LADDER_BREAK, SoundEvents::LADDER_STEP, SoundEvents::LADDER_PLACE,
                           SoundEvents::LADDER_HIT, SoundEvents::LADDER_FALL);
    ANVIL = new SoundType(0.3f, 1.0f, SoundEvents::ANVIL_BREAK, SoundEvents::ANVIL_STEP, SoundEvents::ANVIL_PLACE,
                          SoundEvents::ANVIL_HIT, SoundEvents::ANVIL_FALL);
    SLIME = new SoundType(1.0f, 1.0f, SoundEvents::SLIME_BREAK, SoundEvents::SLIME_STEP, SoundEvents::SLIME_PLACE,
                          SoundEvents::SLIME_HIT, SoundEvents::SLIME_FALL);
}

float SoundType::getVolume() const
{
    return volume;
}

float SoundType::getPitch() const
{
    return pitch;
}

const SoundEvent* SoundType::getBreakSound() const
{
    return breakSound;
}

const SoundEvent* SoundType::getStepSound() const
{
    return stepSound;
}

const SoundEvent* SoundType::getPlaceSound() const
{
    return placeSound;
}

const SoundEvent* SoundType::getHitSound() const
{
    return hitSound;
}

const SoundEvent* SoundType::getFallSound() const
{
    return fallSound;
}
