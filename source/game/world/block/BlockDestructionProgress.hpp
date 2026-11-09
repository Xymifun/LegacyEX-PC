#ifndef GAME_WORLD_BLOCK_BLOCKDESTRUCTIONPROGRESS_HPP
#define GAME_WORLD_BLOCK_BLOCKDESTRUCTIONPROGRESS_HPP

#include "game/world/block/BlockPos.hpp"

class BlockDestructionProgress
{
public:
    BlockDestructionProgress(int id, const BlockPos& pos);

    int getId();
    BlockPos getPos();
    void setProgress(int progress);
    int getProgress();
    void updateTick(int tick);
    int getUpdatedRenderTick();

private:
    int mId;
    BlockPos mPos;
    int mProgress;
    int mUpdatedRenderTick;
};

#endif
