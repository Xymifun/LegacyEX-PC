#ifndef GAME_WORLD_BLOCK_DYNAMICLIQUIDBLOCK_HPP
#define GAME_WORLD_BLOCK_DYNAMICLIQUIDBLOCK_HPP

#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/BlockPos.hpp"

#include <deque>

class BlockPos;
class BlockState;
class Direction;
class Level;
class LevelSource;
class Material;
class Random;

class DynamicLiquidBlock : public LiquidBlock
{
public:
    struct _LiquidTickData
    {
        Level* level;
        BlockPos pos;
        const BlockState* state;
        Random* random;

        _LiquidTickData(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
            : level(level),
              pos(pos),
              state(state),
              random(random)
        {
        }
    };

    DynamicLiquidBlock(Material* material);

    void setStatic(Level* level, const BlockPos& pos, bool extraLayer);
    int getHighest(Level* level, const BlockPos& pos, int currentHighest);
    bool _isWaterBlocking(Level* level, const BlockPos& pos, const BlockPos& sourcePos, const Direction* direction);
    bool _canSpreadTo(Level* level, const BlockPos& pos, const BlockPos& sourcePos, const Direction* direction);
    void _spread(Level* level, const BlockPos& pos, int depth, bool extraLayer);
    void _trySpreadTo(Level* level, const BlockPos& pos, const BlockPos& sourcePos, const Direction* direction,
                      int depth);
    int _getSlopeDistance(Level* level, const BlockPos& pos, int distance, int directionIndex);
    void _getSpread(Level* level, const BlockPos& pos, bool (&spread)[4]);
    void maintick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    void iterativetick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);

private:
    std::deque<_LiquidTickData> mTickQueue;
    bool mProcessingTicks;
    int mAdjacentSources;
};

#endif
