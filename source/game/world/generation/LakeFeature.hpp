#ifndef GAME_WORLD_GENERATION_LAKEFEATURE_HPP
#define GAME_WORLD_GENERATION_LAKEFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class LakeFeature : public Feature
{
public:
    LakeFeature(Block* block);
    virtual ~LakeFeature() {}
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBlock;
};

#endif
