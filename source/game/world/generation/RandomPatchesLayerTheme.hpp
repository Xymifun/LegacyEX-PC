#ifndef GAME_WORLD_GENERATION_RANDOMPATCHESLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_RANDOMPATCHESLAYERTHEME_HPP

#include "game/world/generation/LayerTheme.hpp"

class BlockState;
class BlockTransaction;

class RandomPatchesLayerTheme : public LayerTheme
{
public:
    RandomPatchesLayerTheme(unsigned int uSeed, double dNoiseScale);

    virtual void Apply(Random& random, BlockTransaction* pTransaction, int& iMinY, int& iMaxY) const;
    virtual const BlockState* BlockAt(Random& random, float fNoise, int iHeightAboveSurface, bool bIsAir) const = 0;

    static int ABOVE_SURFACE(int iDistance);
    static int BELOW_SURFACE(int iDistance);
    static bool IsSolidEnough(const BlockState* pState);
    static int HighestSolidBlock(BlockTransaction* pTransaction, int iX, int iZ);

    unsigned int m_uSeed;
    double m_dNoiseScale;
};

#endif
