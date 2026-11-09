#include "game/entity/mob/EntityTickCache.hpp"

#include "game/client/render/Renderer.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/LevelChunk.hpp"

EntityTickCache::EntityTickCache()
{
    mMinX = 1;
    mMinY = 1;
    mMinZ = 1;
    mMaxX = -1;
    mMaxY = -1;
    mMaxZ = -1;
    mAccessedMinX = 0;
    mAccessedMinY = 0;
    mAccessedMinZ = 0;
    mAccessedMaxX = 0;
    mAccessedMaxY = 0;
    mAccessedMaxZ = 0;
    mLastCenterX = 0;
    mLastCenterY = 0;
    mLastCenterZ = 0;
    mElements = NULL;
    mBlockStates = NULL;
    mLastFrameUpdated = 0;
    mCacheChanged = true;
}

EntityTickCache::~EntityTickCache()
{
    delete mElements;
    delete mBlockStates;
}

int EntityTickCache::getIndexInCache(int x, int y, int z) const
{
    int depth = mMaxZ - mMinZ + 1;
    int height = mMaxY - mMinY + 1;

    return (x - mMinX) * (height * depth) + (y - mMinY) * depth + (z - mMinZ);
}

void EntityTickCache::blockChanged(const BlockPos& pos)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    if (x >= mMinX && x <= mMaxX && y >= mMinY && y <= mMaxY && z >= mMinZ && z <= mMaxZ)
    {
        int index = getIndexInCache(x, y, z);
        unsigned short element = mLevel->getElement(x, y, z);

        const BlockState* state = Block::getStateByElementId(element);

        mBlockStates[index] = state;
        mElements[index] = element;
    }
}

void EntityTickCache::updateExtents(int x, int y, int z)
{
    if (x < mAccessedMinX)
    {
        mAccessedMinX = x;
    }

    if (y < mAccessedMinY)
    {
        mAccessedMinY = y;
    }

    if (z < mAccessedMinZ)
    {
        mAccessedMinZ = z;
    }

    if (x > mAccessedMaxX)
    {
        mAccessedMaxX = x;
    }

    if (y > mAccessedMaxY)
    {
        mAccessedMaxY = y;
    }

    if (z > mAccessedMaxZ)
    {
        mAccessedMaxZ = z;
    }
}

unsigned short EntityTickCache::getElement(int x, int y, int z)
{
    updateExtents(x, y, z);

    if (x < mMinX || x > mMaxX || y < mMinY || y > mMaxY || z < mMinZ || z > mMaxZ)
    {
        return 0xFFFF;
    }

    int index = getIndexInCache(x, y, z);

    if (mElements[index] == 0xFFFF)
    {
        mElements[index] = mLevel->getElement(x, y, z);
    }

    return mElements[index];
}

const BlockState* EntityTickCache::getBlockState(int x, int y, int z)
{
    updateExtents(x, y, z);

    if (x < mMinX || x > mMaxX || y < mMinY || y > mMaxY || z < mMinZ || z > mMaxZ)
    {
        return NULL;
    }

    int index = getIndexInCache(x, y, z);

    if (mBlockStates[index] == NULL)
    {
        mBlockStates[index] = Block::getStateByElementId(getElement(x, y, z));
    }

    return mBlockStates[index];
}

void EntityTickCache::clearCache()
{
    mMinX = 0;
    mMinY = 0;
    mMinZ = 0;
    mMaxX = 0;
    mMaxY = 0;
    mMaxZ = 0;

    delete mBlockStates;
    mBlockStates = NULL;

    delete mElements;
    mElements = NULL;
}

void EntityTickCache::buildCache(Level* level, int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    mLevel = level;

    int centerX = (minX + maxX) / 2;
    int centerY = (minY + maxY) / 2;
    int centerZ = (minZ + maxZ) / 2;

    if (mAccessedMinX < minX)
    {
        minX = mAccessedMinX;
    }

    if (centerX - minX > 3)
    {
        minX = centerX - 3;
    }

    if (mAccessedMinY < minY)
    {
        minY = mAccessedMinY;
    }

    if (centerY - minY > 3)
    {
        minY = centerY - 3;
    }

    if (mAccessedMinZ < minZ)
    {
        minZ = mAccessedMinZ;
    }

    if (centerZ - minZ > 3)
    {
        minZ = centerZ - 3;
    }

    if (mAccessedMaxX > maxX)
    {
        maxX = mAccessedMaxX;
    }

    if (maxX - centerX > 3)
    {
        maxX = centerX + 3;
    }

    if (mAccessedMaxY > maxY)
    {
        maxY = mAccessedMaxY;
    }

    if (maxY - centerY > 3)
    {
        maxY = centerY + 3;
    }

    if (mAccessedMaxZ > maxZ)
    {
        maxZ = mAccessedMaxZ;
    }

    if (maxZ - centerZ > 3)
    {
        maxZ = centerZ + 3;
    }

    unsigned short* elements = NULL;
    const BlockState** blockStates = NULL;
    bool chunksChanged = false;

    int maxCachedX = mMaxX;
    int minChunkZ = mMinZ >> 4;
    int maxChunkZ = mMaxZ >> 4;

    for (int chunkX = mMinX >> 4; chunkX <= maxCachedX >> 4; ++chunkX)
    {
        for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
        {
            if (level->getChunk(chunkX, chunkZ)->getElementFrameLastUpdated() >= mLastFrameUpdated)
            {
                chunksChanged = true;
            }
        }
    }

    bool boundsChanged = true;

    if (mMinX == minX && mMinY == minY && mMinZ == minZ && mMaxX == maxX && mMaxY == maxY && mMaxZ == maxZ)
    {
        boundsChanged = false;
    }

    if (!chunksChanged && !boundsChanged)
    {
        mCacheChanged = false;

        return;
    }

    unsigned int size = (maxX - minX + 1) * (maxY - minY + 1) * (maxZ - minZ + 1);

    mCacheChanged = true;

    if (size >= 1025)
    {
        clearCache();

        return;
    }

    if ((int)size > 0)
    {
        elements = new unsigned short[size];
        blockStates = new const BlockState*[size];
    }

    int index = 0;

    for (int x = minX; x <= maxX; ++x)
    {
        for (int y = minY; y <= maxY; ++y)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                if (chunksChanged)
                {
                    elements[index] = 0xFFFF;
                    blockStates[index] = NULL;
                }
                else
                {
                    unsigned short element = getElement(x, y, z);
                    const BlockState* state = getBlockState(x, y, z);

                    elements[index] = element;
                    blockStates[index] = state;
                }

                ++index;
            }
        }
    }

    mMinX = minX;
    mMinY = minY;
    mMinZ = minZ;
    mMaxX = maxX;
    mMaxY = maxY;
    mMaxZ = maxZ;

    if (centerX != mLastCenterX || centerY != mLastCenterY || centerZ != mLastCenterZ)
    {
        mAccessedMinX = mMaxX;
        mAccessedMinY = mMaxY;
        mAccessedMinZ = mMaxZ;
        mAccessedMaxX = mMinX;
        mAccessedMaxY = mMinY;
        mAccessedMaxZ = mMinZ;
    }

    mLastCenterX = centerX;
    mLastCenterY = centerY;
    mLastCenterZ = centerZ;

    if (mElements != NULL)
    {
        delete mElements;
    }

    mElements = elements;

    if (mBlockStates != NULL)
    {
        delete mBlockStates;
    }

    mBlockStates = blockStates;

    mLastFrameUpdated = gRenderer->GetFramesSinceBoot();
}

void EntityTickCache::buildCache(Level* level, AABB* box)
{
    int minX = (int)box->mMinX;

    if (box->mMinX < (double)minX)
    {
        --minX;
    }

    int minY = (int)box->mMinY;

    if (box->mMinY < (double)minY)
    {
        --minY;
    }

    int minZ = (int)box->mMinZ;

    if (box->mMinZ < (double)minZ)
    {
        --minZ;
    }

    float x1 = (float)box->mMaxX;
    int maxX = (int)x1;

    if (x1 > (float)maxX)
    {
        ++maxX;
    }

    float y1 = (float)box->mMaxY;
    int maxY = (int)y1;

    if (y1 > (float)maxY)
    {
        ++maxY;
    }

    float z1 = (float)box->mMaxZ;
    int maxZ = (int)z1;

    if (z1 > (float)maxZ)
    {
        ++maxZ;
    }

    buildCache(level, minX, minY, minZ, maxX, maxY, maxZ);
}
