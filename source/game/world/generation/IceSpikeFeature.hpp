#ifndef GAME_WORLD_GENERATION_ICESPIKEFEATURE_HPP
#define GAME_WORLD_GENERATION_ICESPIKEFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class IceSpikeFeature : public Feature
{
public:
    virtual ~IceSpikeFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
