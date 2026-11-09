#include "game/core/GenerationWriteTask.hpp"

#include <algorithm>
#include <deque>

#include "game/core/System.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

GenerationWriteTask::GenerationWriteTask(Level* level, BlockTransaction* transaction, const BlockPos& origin,
                                         int* changedBlocks)
    : mLevel(level),
      mTransaction(transaction),
      mChunks(transaction->getAffectedChunks(level, origin)),
      mOrigin(origin),
      mChangedBlocks(changedBlocks)
{
}

GenerationWriteTask::~GenerationWriteTask()
{
    delete mTransaction;
    delete mChunks;
}

void GenerationWriteTask::doWork(long long endTime)
{
    if (mLevel->mChunkSource != NULL)
    {
        while (!mChunks->empty() && System::processTimeInMilliSecs() < endTime)
        {
            const ChunkPos& front = mChunks->front();
            ChunkPos chunk = front;
            mChunks->pop_front();

            if (mLevel->getChunk(chunk.x, chunk.z) == NULL)
            {
                Sleep(1);
                mChunks->push_back(chunk);
                continue;
            }

            if (mLevel->mIsClientSide)
            {
                if (!mLevel->reallyHasChunk(chunk.x, chunk.z))
                {
                    Sleep(1);
                    mChunks->push_back(chunk);
                    continue;
                }
            }

            if (!mLevel->mIsClientSide)
            {
                if (!static_cast<ServerLevel*>(mLevel)->getChunkSource()->hasChunk(chunk.x, chunk.z))
                {
                    Sleep(1);
                    mChunks->push_back(chunk);
                    continue;
                }
            }

            BlockPos min = mOrigin.offset(mTransaction->getMin());
            BlockPos max = mOrigin.offset(mTransaction->getMax());
            BlockPos from(std::max(chunk.x << 4, min.x), min.y, std::max(chunk.z << 4, min.z));
            BlockPos to(std::min(chunk.x * 16 + 16, max.x), max.y, std::min(chunk.z * 16 + 16, max.z));

            for (int x = from.x; x < to.x; ++x)
            {
                for (int z = from.z; z < to.z; ++z)
                {
                    for (int y = from.y; y < to.y; ++y)
                    {
                        Vec3i position = Vec3i(x, y, z).subtract(mOrigin);
                        if (mTransaction->inBounds(position) && mTransaction->shouldWrite(position))
                        {
                            int block = mTransaction->getBlock(position);
                            unsigned short data = mTransaction->getData(position);
                            if (block == BlockId::STRUCTURE_VOID)
                                block = BlockId::AIR;

                            bool countsAsChanged = block != BlockId::AIR && block != BlockId::VINE
                                                   && block != BlockId::TNT && block != BlockId::STONE_PRESSURE_PLATE
                                                   && block != BlockId::FIRE && block != BlockId::TALLGRASS;
                            if (mChangedBlocks != NULL && countsAsChanged)
                                ++*mChangedBlocks;
                            mLevel->setBlockAndData(x, y, z, block, data, Block::UPDATE_CLIENTS);
                        }
                    }
                }
            }

            mLevel->setBlocksDirty(from, to);
        }
    }
}

bool GenerationWriteTask::hasFinished() const
{
    return mChunks->empty();
}
