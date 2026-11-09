#ifndef GAME_WORLD_GENERATION_SEAANEMONEFEATURE_HPP
#define GAME_WORLD_GENERATION_SEAANEMONEFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class SeaAnemoneFeature : public Feature
{
public:
    virtual ~SeaAnemoneFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
