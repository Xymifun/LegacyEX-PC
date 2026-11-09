#include "game/world/generation/BasicPatchesLayerTheme.hpp"

#include <algorithm>

#include "game/util/Random.hpp"

BasicPatchesLayerTheme::BasicPatchesLayerTheme(unsigned int seed, double noiseScale, int depth,
                                               const std::vector<const BlockState*>& blocks, float noiseThreshold)
    : RandomPatchesLayerTheme(seed, noiseScale),
      mNoiseThreshold(noiseThreshold),
      mDepth(depth),
      mBlocks(blocks)
{
}

const BlockState* BasicPatchesLayerTheme::BlockAt(Random&, float noise, int heightAboveSurface, bool isAir) const
{
    if (noise <= mNoiseThreshold && heightAboveSurface <= mDepth)
    {
        int index = std::min(static_cast<int>(mBlocks.size()) - 1, mDepth - heightAboveSurface);
        return mBlocks.at(index);
    }
    return 0;
}
