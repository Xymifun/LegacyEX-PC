#ifndef GAME_WORLD_GENERATION_PUMPKINFEATURE_HPP
#define GAME_WORLD_GENERATION_PUMPKINFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class PumpkinFeature : public Feature
{
public:
    virtual ~PumpkinFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
