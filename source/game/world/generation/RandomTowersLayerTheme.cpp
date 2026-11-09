#include <cstddef>

#include "game/world/generation/RandomTowersLayerTheme.hpp"

RandomTowersLayerTheme::RandomTowersLayerTheme(unsigned int uSeed, double dNoiseScale, double dTowerThreshold,
                                               const std::vector<const BlockState*>& towerBlocks)
    : RandomPatchesLayerTheme(uSeed, dNoiseScale),
      m_dTowerThreshold(dTowerThreshold),
      m_apTowerBlocks(towerBlocks)
{
}

const BlockState* RandomTowersLayerTheme::BlockAt(Random& random, float fNoise, int iHeightAboveSurface,
                                                  bool bIsAir) const
{
    if (fNoise < m_dTowerThreshold && iHeightAboveSurface <= 1)
    {
        unsigned int uIndex = 1 - iHeightAboveSurface;
        if (uIndex < m_apTowerBlocks.size())
        {
            return m_apTowerBlocks[uIndex];
        }
    }

    return NULL;
}
