#ifndef GAME_WORLD_BLOCK_SIMPLEBLOCKREMOVER_HPP
#define GAME_WORLD_BLOCK_SIMPLEBLOCKREMOVER_HPP

#include <vector>

#include "game/world/block/BlockRemover.hpp"

class AABB;
class Level;

class SimpleBlockRemover : public BlockRemover
{
public:
    SimpleBlockRemover(Level* level, std::vector<AABB*>& areas);

    virtual bool RemoveBlocks(int areaIndex, int maxBlocks);
};

#endif
