#ifndef GAME_WORLD_CHUNK_ACTIVECHUNKBOUNDSCHECKER_HPP
#define GAME_WORLD_CHUNK_ACTIVECHUNKBOUNDSCHECKER_HPP

#include <vector>

class AABB;

class ActiveChunkBoundsChecker
{
public:
    ActiveChunkBoundsChecker();
    ~ActiveChunkBoundsChecker();

    void ActivateCache();
    void DeactivateCache();
    void ClearCache();

    bool IsChunkWithinActiveArea(int chunkX, int chunkZ, const std::vector<AABB*>& activeBounds);

private:
    enum
    {
        CHUNK_CACHE_RADIUS = 27,
        CHUNK_CACHE_DIAMETER = CHUNK_CACHE_RADIUS * 2,
        CHUNK_CACHE_BYTES = (CHUNK_CACHE_DIAMETER * CHUNK_CACHE_DIAMETER + 7) / 8,
        CHUNK_CACHE_BIT_COUNT = CHUNK_CACHE_BYTES * 8
    };

    unsigned char* mCacheEvaluated;
    unsigned char* mCacheResult;
    int mCacheBitCount;
    bool mCacheActive;
};

#endif
