#ifndef GAME_WORLD_BLOCK_BLOCKPATTERN_HPP
#define GAME_WORLD_BLOCK_BLOCKPATTERN_HPP

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/chunk/array3DWithLength.hpp"

class BlockInWorld;
class Direction;
class Level;


class BlockPattern
{
public:
    class BlockLoadingCache
    {
    public:
        BlockLoadingCache(Level* level, bool loadChunks);
        ~BlockLoadingCache();

        BlockInWorld* getUnchecked(const BlockPos& pos);

    private:
        typedef boost::unordered_map<BlockPos, BlockInWorld*, boost::hash<BlockPos>, std::equal_to<BlockPos>,
                                     std::allocator<std::pair<const BlockPos, BlockInWorld*> > >
            BlockCache;

        Level* mLevel;
        bool mLoadChunks;
        BlockCache mCache;
    };

    class BlockPatternMatch
    {
    public:
        BlockPatternMatch(const BlockPos& frontTopLeft, const Direction* forwards, const Direction* up,
                          boost::shared_ptr<BlockLoadingCache> cache, int width, int height, int depth);
        ~BlockPatternMatch();

        BlockPos getFrontTopLeft();
        const Direction* getForwards();
        const Direction* getUp();
        int getWidth();
        int getHeight();
        int getDepth();
        BlockInWorld* getBlock(int x, int y, int z);

    private:
        BlockPos mFrontTopLeft;
        const Direction* mForwards;
        const Direction* mUp;
        boost::shared_ptr<BlockLoadingCache> mCache;
        int mWidth;
        int mHeight;
        int mDepth;
    };

    BlockPattern(array3DWithLength<Predicate<BlockInWorld*>*> pattern, const Direction* up, const Direction* forward);

    unsigned int getHeight();
    unsigned int getWidth();
    const Direction* getUpRelativeToPattern();
    const Direction* getForwardRelativeToPattern();
    unsigned int getDepth();
    array3DWithLength<Predicate<BlockInWorld*>*> getPattern();
    std::wstring toString();

    static BlockLoadingCache* createLevelCache(Level* level, bool loadChunks);
    static BlockPos translateAndRotate(const BlockPos& pos, const Direction* forwards, const Direction* up, int x,
                                       int y, int z);

    BlockPatternMatch* matches(const BlockPos& pos, const Direction* forwards, const Direction* up,
                               boost::shared_ptr<BlockLoadingCache> cache);
    BlockPatternMatch* matches(Level* level, const BlockPos& pos, const Direction* forwards, const Direction* up);
    BlockPatternMatch* find(Level* level, const BlockPos& pos);

private:
    array3DWithLength<Predicate<BlockInWorld*>*> mPattern;
    int mDepth;
    int mHeight;
    int mWidth;
    const Direction* mUp;
    const Direction* mForward;
};

#endif
