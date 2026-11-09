#ifndef GAME_CORE_GENERATIONWRITETASK_HPP
#define GAME_CORE_GENERATIONWRITETASK_HPP

#include <deque>

#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockTransaction.hpp"

class ChunkPos;

class GenerationWriteTask
{
public:
    Level* mLevel;
    BlockTransaction* mTransaction;
    std::deque<ChunkPos>* mChunks;
    BlockPos mOrigin;
    int* mChangedBlocks;

    GenerationWriteTask(Level* level, BlockTransaction* transaction, const BlockPos& origin, int* changedBlocks);
    ~GenerationWriteTask();

    void doWork(long long endTime);
    bool hasFinished() const;
};

#endif
