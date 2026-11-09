#ifndef GAME_WORLD_GENERATION_MELONFEATURE_HPP
#define GAME_WORLD_GENERATION_MELONFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class MelonFeature : public Feature
{
public:
    MelonFeature();
    virtual ~MelonFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
