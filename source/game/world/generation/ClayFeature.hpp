#ifndef GAME_WORLD_GENERATION_CLAYFEATURE_HPP
#define GAME_WORLD_GENERATION_CLAYFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class ClayFeature : public Feature
{
public:
    ClayFeature(int numberOfBlocks);
    virtual ~ClayFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mClay;
    int mNumberOfBlocks;
};

#endif
