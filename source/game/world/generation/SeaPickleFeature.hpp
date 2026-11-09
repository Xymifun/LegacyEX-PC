#ifndef GAME_WORLD_GENERATION_SEAPICKLEFEATURE_HPP
#define GAME_WORLD_GENERATION_SEAPICKLEFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class SeaPickleFeature : public Feature
{
public:
    virtual ~SeaPickleFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
