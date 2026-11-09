#ifndef GAME_WORLD_GENERATION_HELLSPRINGFEATURE_HPP
#define GAME_WORLD_GENERATION_HELLSPRINGFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class HellSpringFeature : public Feature
{
public:
    HellSpringFeature(Block* block, bool insideRock);
    virtual ~HellSpringFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBlock;
    bool mInsideRock;
};

#endif
