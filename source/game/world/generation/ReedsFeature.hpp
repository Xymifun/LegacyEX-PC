#ifndef GAME_WORLD_GENERATION_REEDSFEATURE_HPP
#define GAME_WORLD_GENERATION_REEDSFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class ReedsFeature : public Feature
{
public:
    virtual ~ReedsFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
