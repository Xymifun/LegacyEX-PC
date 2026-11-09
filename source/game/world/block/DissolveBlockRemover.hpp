#ifndef GAME_WORLD_BLOCK_DISSOLVEBLOCKREMOVER_HPP
#define GAME_WORLD_BLOCK_DISSOLVEBLOCKREMOVER_HPP

#include <vector>

#include "game/world/block/BlockRemover.hpp"

class AABB;
class Level;

class DissolveBlockRemover : public BlockRemover
{
public:
    DissolveBlockRemover(Level* level, std::vector<AABB*>& areas, int passCount, int ticksPerRemoval);

    virtual void Tick();
    virtual bool RemoveBlocks(int areaIndex, int maxBlocks);

private:
    int mPassCount;
};

#endif
