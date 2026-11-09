#ifndef GAME_WORLD_GENERATION_RANDOMTOWERSLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_RANDOMTOWERSLAYERTHEME_HPP

#include <vector>

#include "game/world/generation/RandomPatchesLayerTheme.hpp"

class RandomTowersLayerTheme : public RandomPatchesLayerTheme
{
public:
    RandomTowersLayerTheme(unsigned int uSeed, double dNoiseScale, double dTowerThreshold,
                           const std::vector<const BlockState*>& towerBlocks);

    virtual const BlockState* BlockAt(Random& random, float fNoise, int iHeightAboveSurface, bool bIsAir) const;

    double m_dTowerThreshold;
    std::vector<const BlockState*> m_apTowerBlocks;
};

#endif
