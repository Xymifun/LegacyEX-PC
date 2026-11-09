#ifndef GAME_WORLD_GENERATION_BLUEICEFEATURE_HPP
#define GAME_WORLD_GENERATION_BLUEICEFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class BlueIceFeature : public Feature
{
public:
    virtual ~BlueIceFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
