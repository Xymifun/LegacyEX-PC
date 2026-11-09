#ifndef GAME_WORLD_GENERATION_SPRUCEFEATURE_HPP
#define GAME_WORLD_GENERATION_SPRUCEFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class Level;
class Random;

class SpruceFeature : public AbstractTreeFeature
{
public:
    SpruceFeature(bool doBlockNotify);

    static void staticCtor();
    virtual bool place(Level* level, Random& random, const BlockPos& pos);
};

#endif
