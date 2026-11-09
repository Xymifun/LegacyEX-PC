#include "game/world/chunk/ActiveChunkBoundsChecker.hpp"

#include "game/util/AABB.hpp"

#include <string.h>

ActiveChunkBoundsChecker::ActiveChunkBoundsChecker()
{
    mCacheBitCount = CHUNK_CACHE_BIT_COUNT;
    mCacheEvaluated = new unsigned char[CHUNK_CACHE_BYTES];
    mCacheResult = new unsigned char[mCacheBitCount / 8];
    memset(mCacheEvaluated, 0, mCacheBitCount / 8);
    memset(mCacheResult, 0, mCacheBitCount / 8);
    mCacheActive = false;
}

ActiveChunkBoundsChecker::~ActiveChunkBoundsChecker()
{
    delete[] mCacheEvaluated;
    delete[] mCacheResult;
}

void ActiveChunkBoundsChecker::ActivateCache()
{
    mCacheActive = true;
}

void ActiveChunkBoundsChecker::DeactivateCache()
{
    mCacheActive = false;
}

void ActiveChunkBoundsChecker::ClearCache()
{
    memset(mCacheEvaluated, 0, mCacheBitCount / 8);
    memset(mCacheResult, 0, mCacheBitCount / 8);
    DeactivateCache();
}

bool ActiveChunkBoundsChecker::IsChunkWithinActiveArea(int chunkX, int chunkZ, const std::vector<AABB*>& activeBounds)
{
    int cacheByte = 0;
    unsigned char cacheMask = 0;
    bool isWithinActiveArea = false;
    bool cacheActive = mCacheActive;

    if (cacheActive)
    {
        int cacheX = chunkX + CHUNK_CACHE_RADIUS;
        int cacheZ = chunkZ + CHUNK_CACHE_RADIUS;
        if (cacheX < 0 || cacheX >= CHUNK_CACHE_DIAMETER || cacheZ < 0 || cacheZ >= CHUNK_CACHE_DIAMETER)
        {
            return false;
        }

        int cacheIndex = cacheX * CHUNK_CACHE_DIAMETER + cacheZ;
        if (cacheIndex < 0 || cacheIndex >= mCacheBitCount)
        {
            return false;
        }

        cacheByte = cacheIndex / 8;
        cacheMask = (unsigned char)(0x80 >> (cacheIndex % 8));

        if ((mCacheEvaluated[cacheByte] & cacheMask) != 0)
        {
            return (mCacheResult[cacheByte] & cacheMask) != 0;
        }
    }

    if (!activeBounds.empty())
    {
        for (std::vector<AABB*>::const_iterator it = activeBounds.begin(), end = activeBounds.end(); it != end; ++it)
        {
            const AABB* bounds = *it;
            isWithinActiveArea = bounds->mMinX <= chunkX && chunkX <= bounds->mMaxX && bounds->mMinZ <= chunkZ
                                 && chunkZ <= bounds->mMaxZ;
            if (isWithinActiveArea)
            {
                break;
            }
        }
    }

    if (cacheActive)
    {
        mCacheEvaluated[cacheByte] |= cacheMask;
        if (isWithinActiveArea)
        {
            mCacheResult[cacheByte] |= cacheMask;
        }
    }

    return isWithinActiveArea;
}
