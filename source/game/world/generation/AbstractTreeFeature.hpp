#ifndef GAME_WORLD_GENERATION_ABSTRACTTREEFEATURE_HPP
#define GAME_WORLD_GENERATION_ABSTRACTTREEFEATURE_HPP

#include "game/world/generation/Feature.hpp"

class Block;
class BlockPos;
class BlockState;
class BooleanProperty;
class Level;
class Random;

class AbstractTreeFeature : public Feature
{
public:
    AbstractTreeFeature(bool doBlockNotify, int baseHeight, const BlockState* trunkState, const BlockState* leafState,
                        bool hasVines);
    virtual ~AbstractTreeFeature();

    bool isFree(Block* block);
    virtual void postPlaceTree(Level* level, Random& random, const BlockPos& pos);
    void setDirtAt(Level* level, const BlockPos& pos);
    void placeFallenTrunk(Level* level, const BlockPos& pos, Random& random, int length);
    void addVine(Level* level, const BlockPos& pos, const BooleanProperty* property);
    bool placeTrunk(Level* level, const BlockPos& pos, Random& random, int height);

protected:
    int mBaseHeight;
    bool mHasVines;
    const BlockState* mTrunkState;
    const BlockState* mLeafState;
};

#endif
