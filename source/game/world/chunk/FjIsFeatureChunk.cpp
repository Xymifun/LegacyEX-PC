#include "game/world/chunk/FjIsFeatureChunk.hpp"

#include <boost/unordered_set.hpp>

#include "game/util/Random.hpp"
#include "game/world/biome/BiomeSource.hpp"

FjIsFeatureChunk::FjIsFeatureChunk()
{
    mMaxAttempts = 0;
    mSpacing = 0;
    mSeparation = 0;
    mBiomeRequirements = NULL;
}

FjIsFeatureChunk::FjIsFeatureChunk(const FjFeatureBiomeRequirements* biomeRequirements, unsigned int maxAttempts,
                                   int spacing, int separation)
{
    mMaxAttempts = maxAttempts;
    mSpacing = spacing;
    mSeparation = separation;
    mBiomeRequirements = biomeRequirements;
}

ChunkPos FjIsFeatureChunk::getCenterChunk(int chunkX, int chunkZ) const
{
    if (chunkX < 0)
    {
        chunkX = chunkX - (mSpacing - 1);
    }

    if (chunkZ < 0)
    {
        chunkZ = chunkZ - (mSpacing - 1);
    }

    return ChunkPos((chunkX / mSpacing) * mSpacing, (chunkZ / mSpacing) * mSpacing);
}

bool FjIsFeatureChunk::couldSpawnInChunk(BiomeSource* biomeSource, int sizeInChunks, int chunkX, int chunkZ) const
{
    int borderInChunks = sizeInChunks / 2 - 3;

    return -borderInChunks <= chunkX && chunkX < borderInChunks && -borderInChunks <= chunkZ && chunkZ < borderInChunks
           && biomeSource->fjCouldSpawn(mBiomeRequirements, chunkX, chunkZ);
}

bool FjIsFeatureChunk::isFeatureChunk(Random& random, BiomeSource* biomeSource, int sizeInChunks, int chunkX,
                                      int chunkZ) const
{
    int range = mSpacing - mSeparation;

    unsigned int maxAttempts = mMaxAttempts < (unsigned int)(range * range) ? mMaxAttempts
                                                                            : (unsigned int)(range * range);

    if (couldSpawnInChunk(biomeSource, sizeInChunks, chunkX, chunkZ))
    {
        ChunkPos centerChunk = getCenterChunk(chunkX, chunkZ);

        boost::unordered_set<ChunkPos, ChunkPosKeyHash, ChunkPosKeyEq> attemptedChunks(11);

        while (attemptedChunks.size() < maxAttempts)
        {
            ChunkPos attemptedChunk = centerChunk;
            attemptedChunk.x += random.nextInt(range);
            attemptedChunk.z += random.nextInt(range);

            bool inserted = attemptedChunks.insert(attemptedChunk).second;

            if (inserted)
            {
                if (chunkX == attemptedChunk.x && chunkZ == attemptedChunk.z)
                {
                    return true;
                }

                if (couldSpawnInChunk(biomeSource, sizeInChunks, attemptedChunk.x, attemptedChunk.z))
                {
                    return false;
                }
            }
        }
    }

    return false;
}

bool FjIsFeatureChunk::UseReducedSpacing(int sizeInChunks, bool isSuperflat)
{
    return !isSuperflat && sizeInChunks < 128;
}

FjIsFeatureChunk FjIsFeatureChunk::CreateReduced(const FjIsFeatureChunk& featureChunk)
{
    return FjIsFeatureChunk(featureChunk.mBiomeRequirements, (int)(featureChunk.mMaxAttempts * 1.5), 32,
                            featureChunk.mSeparation);
}
