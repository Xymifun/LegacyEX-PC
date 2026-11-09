#ifndef GAME_WORLD_GENERATION_SIMPLEPATCHESLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_SIMPLEPATCHESLAYERTHEME_HPP

#include "game/world/generation/RandomPatchesLayerTheme.hpp"

class SimplePatchesLayerTheme : public RandomPatchesLayerTheme
{
public:
    SimplePatchesLayerTheme(unsigned int uSeed, double dNoiseScale, const BlockState* pPatchBlock,
                            float fPatchThreshold);

    virtual const BlockState* BlockAt(Random& random, float fNoise, int iHeightAboveSurface, bool bIsAir) const;

    float m_fPatchThreshold;
    const BlockState* m_pPatchBlock;
};

#endif
