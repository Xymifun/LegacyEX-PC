#ifndef GAME_WORLD_BLOCK_SPIRALBLOCKREMOVER_HPP
#define GAME_WORLD_BLOCK_SPIRALBLOCKREMOVER_HPP

#include <vector>

#include "game/core/Spiral.hpp"
#include "game/world/block/BlockRemover.hpp"

class AABB;
class Level;

class SpiralBlockRemover : public BlockRemover
{
public:
    SpiralBlockRemover(Level* level, std::vector<AABB*>& areas);

    virtual void Tick();
    virtual bool RemoveBlocks(int areaIndex, int maxBlocks);

private:
    std::vector<Spiral> mSpirals;
};

#endif
