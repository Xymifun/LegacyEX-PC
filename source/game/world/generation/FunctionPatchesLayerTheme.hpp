#ifndef GAME_WORLD_GENERATION_FUNCTIONPATCHESLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_FUNCTIONPATCHESLAYERTHEME_HPP

#include "game/world/generation/RandomPatchesLayerTheme.hpp"

class BlockState;

class FunctionPatchesLayerTheme : public RandomPatchesLayerTheme
{
public:
    typedef const BlockState* (*BlockFunction)(Random&, float, int, bool);

    FunctionPatchesLayerTheme(unsigned int seed, double noiseScale, BlockFunction blockFunction);

    static const BlockState* GRASS_PATCHES(Random& random, float noise, int heightAboveSurface, bool isAir);
    static const BlockState* HELLFIRE_PATCHES(Random& random, float noise, int heightAboveSurface, bool isAir);
    static const BlockState* SNOWLAYER_PATCHES(Random& random, float noise, int heightAboveSurface, bool isAir);
    static const BlockState* TNT_TRAPS(Random& random, float noise, int heightAboveSurface, bool isAir);
    static const BlockState* CARPET_TRAP(Random& random, float noise, int heightAboveSurface, bool isAir);

    virtual const BlockState* BlockAt(Random& random, float noise, int heightAboveSurface, bool isAir) const;

    BlockFunction mBlockFunction;
};

#endif
