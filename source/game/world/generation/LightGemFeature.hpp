#ifndef GAME_WORLD_GENERATION_LIGHTGEMFEATURE_HPP
#define GAME_WORLD_GENERATION_LIGHTGEMFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class LightGemFeature : public Feature
{
public:
    virtual ~LightGemFeature() {}
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
