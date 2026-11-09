#include <cstddef>

#include "game/world/generation/CarpetTrapLayerTheme.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

CarpetTrapLayerTheme::CarpetTrapLayerTheme(unsigned int uSeed, double dNoiseScale, int iCarpetColour)
    : RandomPatchesLayerTheme(uSeed, dNoiseScale),
      m_iCarpetColour(iCarpetColour)
{
}

const BlockState* CarpetTrapLayerTheme::BlockAt(Random& random, float fNoise, int iHeightAboveSurface,
                                                bool bIsAir) const
{
    if (fNoise <= 0.11f)
    {
        if (iHeightAboveSurface == ABOVE_SURFACE(1))
        {
            return Blocks::carpet->getBlockState(m_iCarpetColour);
        }

        if (fNoise < -0.43f && iHeightAboveSurface <= 0)
        {
            return Blocks::air->defaultBlockState();
        }
    }

    return NULL;
}
