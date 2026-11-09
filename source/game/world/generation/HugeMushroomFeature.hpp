#ifndef GAME_WORLD_GENERATION_HUGEMUSHROOMFEATURE_HPP
#define GAME_WORLD_GENERATION_HUGEMUSHROOMFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class HugeMushroomFeature : public Feature
{
public:
    HugeMushroomFeature(Block* block);
    HugeMushroomFeature();
    virtual ~HugeMushroomFeature();

    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBlock;
};

#endif
