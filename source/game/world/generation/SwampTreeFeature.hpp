#ifndef GAME_WORLD_GENERATION_SWAMPTREEFEATURE_HPP
#define GAME_WORLD_GENERATION_SWAMPTREEFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class BooleanProperty;
class Level;
class Random;

class SwampTreeFeature : public AbstractTreeFeature
{
public:
    SwampTreeFeature();
    static void staticCtor();
    void addVine(Level* level, const BlockPos& pos, const BooleanProperty* property);
    virtual bool place(Level* level, Random& random, const BlockPos& pos);
};

#endif
