#ifndef GAME_WORLD_GENERATION_SPRINGFEATURE_HPP
#define GAME_WORLD_GENERATION_SPRINGFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class SpringFeature : public Feature
{
public:
    SpringFeature(Block* block);
    virtual ~SpringFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBlock;
};

#endif
