#ifndef GAME_WORLD_GENERATION_BIRCHFEATURE_HPP
#define GAME_WORLD_GENERATION_BIRCHFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class Level;
class Random;

class BirchFeature : public AbstractTreeFeature
{
public:
    BirchFeature(bool doBlockNotify, bool tallBirch);

    static void staticCtor();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    bool mTallBirch;
};

#endif
