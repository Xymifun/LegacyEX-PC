#ifndef GAME_WORLD_GENERATION_ROOFTREEFEATURE_HPP
#define GAME_WORLD_GENERATION_ROOFTREEFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class Level;
class Random;

class RoofTreeFeature : public AbstractTreeFeature
{
public:
    RoofTreeFeature(bool doBlockNotify);
    virtual ~RoofTreeFeature();

    static void staticCtor();
    bool canPlaceTreeOfHeight(Level* level, const BlockPos& pos, int height);
    void placeLogAt(Level* level, const BlockPos& pos);
    void placeLeafAt(Level* level, int x, int y, int z);
    virtual bool place(Level* level, Random& random, const BlockPos& pos);
};

#endif
