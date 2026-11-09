#ifndef GAME_WORLD_GENERATION_SAVANNATREEFEATURE_HPP
#define GAME_WORLD_GENERATION_SAVANNATREEFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class Level;
class Random;

class SavannaTreeFeature : public AbstractTreeFeature
{
public:
    SavannaTreeFeature(bool doBlockNotify);
    virtual ~SavannaTreeFeature();
    static void staticCtor();
    void placeLogAt(Level* level, const BlockPos& pos);
    void placeLeafAt(Level* level, const BlockPos& pos);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
};

#endif
