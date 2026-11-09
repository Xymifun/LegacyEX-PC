#ifndef GAME_WORLD_GENERATION_TREEFEATURE_HPP
#define GAME_WORLD_GENERATION_TREEFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class BlockState;
class BooleanProperty;
class Direction;
class Level;
class MutableBlockPos;
class Random;

class TreeFeature : public AbstractTreeFeature
{
public:
    TreeFeature(bool doBlockNotify);
    TreeFeature(bool doBlockNotify, int baseHeight, const BlockState* trunkState, const BlockState* leafState,
                bool hasVines);
    virtual ~TreeFeature() {}

    static void staticCtor();
    int prepareSpawn(Level* level, const BlockPos& pos, int height);
    void placeLeaf(Level* level, const MutableBlockPos& pos);
    void addHangingVine(Level* level, const BlockPos& pos, const BooleanProperty* property);
    void placeCocoa(Level* level, int age, const BlockPos& pos, const Direction* direction);
    virtual bool place(Level* level, Random& random, const BlockPos& pos);
};

#endif
