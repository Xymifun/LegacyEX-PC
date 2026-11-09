#ifndef GAME_WORLD_GENERATION_CARPETTRAPLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_CARPETTRAPLAYERTHEME_HPP

#include "game/world/generation/RandomPatchesLayerTheme.hpp"

class CarpetTrapLayerTheme : public RandomPatchesLayerTheme
{
public:
    CarpetTrapLayerTheme(unsigned int uSeed, double dNoiseScale, int iCarpetColour);

    virtual const BlockState* BlockAt(Random& random, float fNoise, int iHeightAboveSurface, bool bIsAir) const;

    int m_iCarpetColour;
};

#endif
