#include "game/world/block/BlockDestructionProgress.hpp"

#include "game/world/block/BlockPos.hpp"

BlockDestructionProgress::BlockDestructionProgress(int id, const BlockPos& pos)
{
    mId = id;
    mPos = pos;
    mProgress = 0;
    mUpdatedRenderTick = 0;
}

int BlockDestructionProgress::getId()
{
    return mId;
}

BlockPos BlockDestructionProgress::getPos()
{
    return mPos;
}

void BlockDestructionProgress::setProgress(int progress)
{
    if (progress > 10)
    {
        progress = 10;
    }

    mProgress = progress;
}

int BlockDestructionProgress::getProgress()
{
    return mProgress;
}

void BlockDestructionProgress::updateTick(int tick)
{
    mUpdatedRenderTick = tick;
}

int BlockDestructionProgress::getUpdatedRenderTick()
{
    return mUpdatedRenderTick;
}
