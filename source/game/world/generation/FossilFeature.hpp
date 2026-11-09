#ifndef GAME_WORLD_GENERATION_FOSSILFEATURE_HPP
#define GAME_WORLD_GENERATION_FOSSILFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class FossilFeature : public Feature
{
public:
    virtual ~FossilFeature() {}
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
