#ifndef GAME_WORLD_CHUNK_FJISFEATURECHUNK_HPP
#define GAME_WORLD_CHUNK_FJISFEATURECHUNK_HPP

#include "game/world/chunk/ChunkPos.hpp"

class BiomeSource;
class FjFeatureBiomeRequirements;
class Random;

class FjIsFeatureChunk
{
public:
    FjIsFeatureChunk();
    FjIsFeatureChunk(const FjFeatureBiomeRequirements* requirements, unsigned int maxAttempts, int spacing,
                     int separation);

    ChunkPos getCenterChunk(int chunkX, int chunkZ) const;
    bool couldSpawnInChunk(BiomeSource* biomeSource, int sizeInChunks, int chunkX, int chunkZ) const;

    virtual bool isFeatureChunk(Random& random, BiomeSource* biomeSource, int sizeInChunks, int chunkX,
                                int chunkZ) const;

    static bool UseReducedSpacing(int sizeInChunks, bool isSuperflat);
    static FjIsFeatureChunk CreateReduced(const FjIsFeatureChunk& featureChunk);

private:
    unsigned int mMaxAttempts;
    int mSpacing;
    int mSeparation;
    const FjFeatureBiomeRequirements* mBiomeRequirements;
};

#endif
