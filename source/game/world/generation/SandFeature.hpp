#ifndef GAME_WORLD_GENERATION_SANDFEATURE_HPP
#define GAME_WORLD_GENERATION_SANDFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class SandFeature : public Feature
{
public:
    SandFeature(Block* block, int numberOfBlocks);
    virtual ~SandFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBlock;
    int mNumberOfBlocks;
};

#endif
