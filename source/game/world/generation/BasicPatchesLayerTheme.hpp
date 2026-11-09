#ifndef GAME_WORLD_GENERATION_BASICPATCHESLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_BASICPATCHESLAYERTHEME_HPP

#include <vector>

#include "game/world/generation/RandomPatchesLayerTheme.hpp"

class BlockState;

class BasicPatchesLayerTheme : public RandomPatchesLayerTheme
{
public:
    BasicPatchesLayerTheme(unsigned int seed, double noiseScale, int depth,
                           const std::vector<const BlockState*>& blocks, float noiseThreshold);
    virtual const BlockState* BlockAt(Random& random, float noise, int heightAboveSurface, bool isAir) const;

private:
    float mNoiseThreshold;
    int mDepth;
    std::vector<const BlockState*> mBlocks;
};

#endif
