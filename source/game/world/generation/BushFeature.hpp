#ifndef GAME_WORLD_GENERATION_BUSHFEATURE_HPP
#define GAME_WORLD_GENERATION_BUSHFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Bush;
class Level;
class Random;

class BushFeature : public Feature
{
public:
    BushFeature(Bush* bush);
    virtual ~BushFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBush;
};

#endif
