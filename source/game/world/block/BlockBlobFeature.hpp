#ifndef GAME_WORLD_BLOCK_BLOCKBLOBFEATURE_HPP
#define GAME_WORLD_BLOCK_BLOCKBLOBFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class BlockBlobFeature : public Feature
{
public:
    BlockBlobFeature(Block* block, int radius);
    virtual ~BlockBlobFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBlock;
    int mRadius;
};

#endif
