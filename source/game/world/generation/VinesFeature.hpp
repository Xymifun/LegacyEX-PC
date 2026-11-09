#ifndef GAME_WORLD_GENERATION_VINESFEATURE_HPP
#define GAME_WORLD_GENERATION_VINESFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class VinesFeature : public Feature
{
public:
    VinesFeature();
    virtual ~VinesFeature() {}

    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
