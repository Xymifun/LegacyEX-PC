#include "game/world/generation/RainbowLayerTheme.hpp"
#include <cmath>

RainbowLayerTheme::RainbowLayerTheme(unsigned int seed, double noiseScale, const std::vector<const BlockState*>& blocks)
    : RandomPatchesLayerTheme(seed, noiseScale),
      mBlocks(blocks)
{
}

const BlockState* RainbowLayerTheme::BlockAt(Random&, float noise, int heightAboveSurface, bool isAir) const
{
    if (heightAboveSurface < 1 && !isAir)
    {
        const int count = static_cast<int>(mBlocks.size());
        const int index = static_cast<int>(std::floor(count * 0.5 * (noise + 1.0f)));
        return mBlocks.at(index);
    }
    return NULL;
}
