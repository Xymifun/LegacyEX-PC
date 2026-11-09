#include <cstddef>

#include "game/world/generation/SimplePatchesLayerTheme.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

SimplePatchesLayerTheme::SimplePatchesLayerTheme(unsigned int uSeed, double dNoiseScale, const BlockState* pPatchBlock,
                                                 float fPatchThreshold)
    : RandomPatchesLayerTheme(uSeed, dNoiseScale),
      m_fPatchThreshold(fPatchThreshold),
      m_pPatchBlock(pPatchBlock)
{
}

const BlockState* SimplePatchesLayerTheme::BlockAt(Random& random, float fNoise, int iHeightAboveSurface,
                                                   bool bIsAir) const
{
    if (fNoise >= m_fPatchThreshold)
    {
        return NULL;
    }

    if (iHeightAboveSurface <= 0)
    {
        if (bIsAir)
        {
            return NULL;
        }

        return m_pPatchBlock;
    }

    return Blocks::air->defaultBlockState();
}
