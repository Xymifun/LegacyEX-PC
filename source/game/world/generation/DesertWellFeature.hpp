#ifndef GAME_WORLD_GENERATION_DESERTWELLFEATURE_HPP
#define GAME_WORLD_GENERATION_DESERTWELLFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class BlockState;
class BlockPos;
class Level;
class Random;

class DesertWellFeature : public Feature
{
public:
    DesertWellFeature();
    virtual ~DesertWellFeature() {}

    static void staticCtor();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    const BlockState* mSlabState;
    const BlockState* mSandstoneState;
    const BlockState* mWaterState;
};

#endif
