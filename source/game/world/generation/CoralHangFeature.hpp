#ifndef GAME_WORLD_GENERATION_CORALHANGFEATURE_HPP
#define GAME_WORLD_GENERATION_CORALHANGFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Direction;
class Level;
class Random;

class CoralHangFeature : public Feature
{
public:
    virtual ~CoralHangFeature();

    bool canPlaceHangOn(Level* level, const BlockPos& pos);
    const Direction* randomDirection(Level* level, Random& random, const BlockPos& pos);
    int turnFacingIntoint(const Direction* direction);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
