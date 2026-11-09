#ifndef GAME_WORLD_GENERATION_DEADBUSHFEATURE_HPP
#define GAME_WORLD_GENERATION_DEADBUSHFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class DeadBushFeature : public Feature
{
public:
    DeadBushFeature(Block* block);
    virtual ~DeadBushFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
