#ifndef GAME_WORLD_BLOCK_BLOCKDEGRADEROUTINE_HPP
#define GAME_WORLD_BLOCK_BLOCKDEGRADEROUTINE_HPP

#include <cstddef>
#include <deque>
#include <string>

#include <boost/unordered_set.hpp>

#include "game/util/AABB.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class DegradationRuleDefinition;
class DegradationSequenceRuleDefinition;
class NamedAreaRuleDefinition;
class ServerLevel;

class BlockDegradeRoutine
{
public:
    class LayerCache
    {
    public:
        LayerCache(ServerLevel* level, NamedAreaRuleDefinition* rule);
        ~LayerCache();

        AABB* getCurrentBounds() const;
        Iterator<BlockPos*>* iterator() const;
        void setBroken(const BlockPos* positions, unsigned int count);
        bool isBroken(const BlockPos& pos);
        bool inLayer(const BlockPos& pos);
        unsigned int getBlockCount() const;

    private:
        ServerLevel* mLevel;
        NamedAreaRuleDefinition* mRule;
        AABB* mCurrentBounds;
        boost::unordered_set<BlockPos, boost::hash<BlockPos>, std::equal_to<BlockPos>, std::allocator<BlockPos> >
            mBroken;
    };

    struct Info
    {
        DegradationRuleDefinition* rule;
        LayerCache* cache;
    };

    typedef void (BlockDegradeRoutine::*Action)(long long, long long);

    BlockDegradeRoutine(ServerLevel* level, DegradationSequenceRuleDefinition* definition);

    void setNextAction(Action action, long long nextActionTime);
    void Action_Wait(long long time, long long ticks);
    void Action_Break(long long time, long long ticks);
    LayerCache* getCacheFor(const std::wstring& name) const;
    void Tick(long long time, long long ticks);

private:
    ServerLevel* mLevel;
    DegradationSequenceRuleDefinition* mDefinition;
    std::deque<Info> mActions;
    int mProgress;
    arrayWithLength<BlockPos> mBlocks;
    long long mNextActionTime;
    Action mAction;
};

#endif
