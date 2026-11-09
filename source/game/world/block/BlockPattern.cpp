#include "game/world/block/BlockPattern.hpp"

#include <algorithm>
#include <vector>

#include "game/core/Direction.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockInWorld.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"

BlockPattern::BlockPattern(array3DWithLength<Predicate<BlockInWorld*>*> pattern, const Direction* up,
                           const Direction* forward)
    : mPattern(pattern),
      mDepth(pattern.length),
      mHeight(pattern.data[0]->length),
      mWidth(pattern.data[0]->data[0]->length),
      mUp(up),
      mForward(forward)
{
}

unsigned int BlockPattern::getHeight()
{
    return mHeight;
}

unsigned int BlockPattern::getWidth()
{
    return mWidth;
}

const Direction* BlockPattern::getUpRelativeToPattern()
{
    return mUp;
}

const Direction* BlockPattern::getForwardRelativeToPattern()
{
    return mForward;
}

BlockPattern::BlockLoadingCache* BlockPattern::createLevelCache(Level* level, bool loadChunks)
{
    return new BlockLoadingCache(level, loadChunks);
}

BlockPos BlockPattern::translateAndRotate(const BlockPos& pos, const Direction* forwards, const Direction* up, int x,
                                          int y, int z)
{
    if (forwards != up && forwards != up->getOpposite())
    {
    }
    Vec3i forward(forwards->getStepX(), forwards->getStepY(), forwards->getStepZ());
    Vec3i upVector(up->getStepX(), up->getStepY(), up->getStepZ());
    Vec3i left = forward.cross(upVector);

    return pos.offset(left.x * x - upVector.x * y + forward.x * z, left.y * x - upVector.y * y + forward.y * z,
                      left.z * x - upVector.z * y + forward.z * z);
}

BlockPattern::BlockPatternMatch* BlockPattern::matches(const BlockPos& pos, const Direction* forwards,
                                                       const Direction* up, boost::shared_ptr<BlockLoadingCache> cache)
{
    for (unsigned int x = 0; x < mWidth; x++)
    {
        for (unsigned int y = 0; y < mHeight; y++)
        {
            for (unsigned int z = 0; z < mDepth; z++)
            {
                Predicate<BlockInWorld*>* predicate = mPattern.data[z]->data[y]->data[x];
                BlockInWorld* block = cache->getUnchecked(translateAndRotate(pos, forwards, up, x, y, z));
                if (!predicate->apply(block))
                {
                    return NULL;
                }
            }
        }
    }

    return new BlockPatternMatch(pos, forwards, up, cache, mWidth, mHeight, mDepth);
}

BlockPattern::BlockPatternMatch* BlockPattern::matches(Level* level, const BlockPos& pos, const Direction* forwards,
                                                       const Direction* up)
{
    boost::shared_ptr<BlockLoadingCache> cache(createLevelCache(level, false));
    return matches(pos, forwards, up, cache);
}

BlockPattern::BlockPatternMatch* BlockPattern::find(Level* level, const BlockPos& pos)
{
    boost::shared_ptr<BlockLoadingCache> cache(createLevelCache(level, false));
    int size = std::max(mDepth, std::max(mHeight, mWidth));

    const Direction* upDirection = getUpRelativeToPattern();
    std::vector<const Direction*>* ups = &Direction::VALUES;
    if (upDirection != NULL)
    {
        ups = new std::vector<const Direction*>();
        ups->push_back(upDirection);
    }

    const Direction* forwardDirection = getForwardRelativeToPattern();
    std::vector<const Direction*>* forwards = &Direction::VALUES;
    if (forwardDirection != NULL)
    {
        forwards = new std::vector<const Direction*>();
        forwards->push_back(forwardDirection);
    }

    Iterator<BlockPos*>* positions = BlockPos::between(pos, pos.offset(size - 1, size - 1, size - 1));
    while (positions->hasNext())
    {
        BlockPos candidate = *positions->next();
        for (std::vector<const Direction*>::iterator forward = forwards->begin(); forward != forwards->end(); ++forward)
        {
            for (std::vector<const Direction*>::iterator up = ups->begin(); up != ups->end(); ++up)
            {
                if (*forward != *up && *forward != (*up)->getOpposite())
                {
                    BlockPatternMatch* match = matches(candidate, *forward, *up, cache);
                    if (match != NULL)
                    {
                        if (upDirection != NULL)
                        {
                            delete ups;
                        }
                        if (forwardDirection != NULL)
                        {
                            delete forwards;
                        }
                        delete positions;
                        return match;
                    }
                }
            }
        }
    }
    if (upDirection != NULL)
    {
        delete ups;
    }
    if (forwardDirection != NULL)
    {
        delete forwards;
    }
    delete positions;
    return NULL;
}

BlockPattern::BlockLoadingCache::BlockLoadingCache(Level* level, bool loadChunks)
    : mLevel(level),
      mLoadChunks(loadChunks)
{
}

BlockPattern::BlockLoadingCache::~BlockLoadingCache()
{
    for (BlockCache::iterator it = mCache.begin(); it != mCache.end(); ++it)
    {
        delete it->second;
    }
}

BlockInWorld* BlockPattern::BlockLoadingCache::getUnchecked(const BlockPos& pos)
{
    BlockCache::iterator it = mCache.find(pos);
    BlockInWorld* block;
    if (it != mCache.end())
    {
        block = it->second;
    }
    else
    {
        block = new BlockInWorld(mLevel, pos, mLoadChunks);
        mCache[pos] = block;
    }
    return block;
}

BlockPattern::BlockPatternMatch::BlockPatternMatch(const BlockPos& frontTopLeft, const Direction* forwards,
                                                   const Direction* up, boost::shared_ptr<BlockLoadingCache> cache,
                                                   int width, int height, int depth)
    : mFrontTopLeft(frontTopLeft),
      mForwards(forwards),
      mUp(up),
      mCache(cache),
      mWidth(width),
      mHeight(height),
      mDepth(depth)
{
}

BlockPattern::BlockPatternMatch::~BlockPatternMatch()
{
}

BlockInWorld* BlockPattern::BlockPatternMatch::getBlock(int x, int y, int z)
{
    return mCache->getUnchecked(BlockPattern::translateAndRotate(mFrontTopLeft, getForwards(), getUp(), x, y, z));
}

BlockPos BlockPattern::BlockPatternMatch::getFrontTopLeft()
{
    return mFrontTopLeft;
}

const Direction* BlockPattern::BlockPatternMatch::getForwards()
{
    return mForwards;
}

const Direction* BlockPattern::BlockPatternMatch::getUp()
{
    return mUp;
}

int BlockPattern::BlockPatternMatch::getWidth()
{
    return mWidth;
}

int BlockPattern::BlockPatternMatch::getHeight()
{
    return mHeight;
}
