#ifndef GAME_WORLD_GENERATION_ICEPATCHFEATURE_HPP
#define GAME_WORLD_GENERATION_ICEPATCHFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class Level;
class Random;

class IcePatchFeature : public Feature
{
public:
    IcePatchFeature(int radius);
    virtual ~IcePatchFeature();

    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    Block* mBlock;
    int mRadius;
};

#endif
