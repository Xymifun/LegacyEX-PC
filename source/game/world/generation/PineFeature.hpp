#ifndef GAME_WORLD_GENERATION_PINEFEATURE_HPP
#define GAME_WORLD_GENERATION_PINEFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class Level;
class Random;

class PineFeature : public AbstractTreeFeature
{
public:
    PineFeature();

    static void staticCtor();
    virtual bool place(Level* level, Random& random, const BlockPos& pos);
};

#endif
