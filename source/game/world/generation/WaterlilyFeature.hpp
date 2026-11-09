#ifndef GAME_WORLD_GENERATION_WATERLILYFEATURE_HPP
#define GAME_WORLD_GENERATION_WATERLILYFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class WaterlilyFeature : public Feature
{
public:
    virtual ~WaterlilyFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
