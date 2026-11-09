#ifndef GAME_WORLD_GENERATION_HELLFIREFEATURE_HPP
#define GAME_WORLD_GENERATION_HELLFIREFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class HellFireFeature : public Feature
{
public:
    virtual ~HellFireFeature() {}
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
