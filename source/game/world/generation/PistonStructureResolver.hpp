#ifndef GAME_WORLD_GENERATION_PISTONSTRUCTURERESOLVER_HPP
#define GAME_WORLD_GENERATION_PISTONSTRUCTURERESOLVER_HPP

#include <vector>

#include "game/world/block/BlockPos.hpp"

class Direction;
class Level;

class PistonStructureResolver
{
public:
    PistonStructureResolver(Level* level, const BlockPos& pistonPos, const Direction* direction, bool extending);

    void reorderListAtCollision(int collisionIndex, int distance);
    bool addBranchingBlocks(const BlockPos& pos);
    bool addBlockLine(const BlockPos& pos, const Direction* direction);
    bool resolve();
    std::vector<BlockPos>& getToPush();
    std::vector<BlockPos>& getToDestroy();
    const Direction* getPushDirection();

private:
    Level* mLevel;
    BlockPos mPistonPos;
    BlockPos mBlockPos;
    const Direction* mMoveDirection;
    std::vector<BlockPos> mToPush;
    std::vector<BlockPos> mToDestroy;
};

#endif
