#ifndef GAME_WORLD_GENERATION_RAINBOWLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_RAINBOWLAYERTHEME_HPP

#include <vector>
#include "game/world/generation/RandomPatchesLayerTheme.hpp"

class BlockState;
class Random;

class RainbowLayerTheme : public RandomPatchesLayerTheme
{
public:
    RainbowLayerTheme(unsigned int seed, double noiseScale, const std::vector<const BlockState*>& blocks);
    virtual const BlockState* BlockAt(Random& random, float noise, int heightAboveSurface, bool isAir) const;

private:
    std::vector<const BlockState*> mBlocks;
};

#endif
