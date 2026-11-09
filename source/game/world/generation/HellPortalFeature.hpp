#ifndef GAME_WORLD_GENERATION_HELLPORTALFEATURE_HPP
#define GAME_WORLD_GENERATION_HELLPORTALFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class HellPortalFeature : public Feature
{
public:
    virtual ~HellPortalFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
