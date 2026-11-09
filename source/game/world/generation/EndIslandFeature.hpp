#ifndef GAME_WORLD_GENERATION_ENDISLANDFEATURE_HPP
#define GAME_WORLD_GENERATION_ENDISLANDFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class EndIslandFeature : public Feature
{
public:
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
