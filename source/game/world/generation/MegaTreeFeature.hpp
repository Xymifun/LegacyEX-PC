#ifndef GAME_WORLD_GENERATION_MEGATREEFEATURE_HPP
#define GAME_WORLD_GENERATION_MEGATREEFEATURE_HPP

#include "game/world/generation/AbstractTreeFeature.hpp"

class BlockPos;
class BlockState;
class Level;
class Random;

class MegaTreeFeature : public AbstractTreeFeature
{
public:
    MegaTreeFeature(bool doBlockNotify, int baseHeight, int extraRandomHeight, const BlockState* trunkState,
                    const BlockState* leafState);
    virtual ~MegaTreeFeature() {}

    virtual int calcTreeHeigth(Random& random);
    virtual bool prepareTree(Level* level, Random& random, const BlockPos& pos, int height);
    virtual void placeDoubleTrunkLeaves(Level* level, const BlockPos& pos, int radius);
    virtual void placeSingleTrunkLeaves(Level* level, const BlockPos& pos, int radius);
    bool checkIsFree(Level* level, const BlockPos& pos, int height);
    bool makeDirtFloor(const BlockPos& pos, Level* level);

protected:
    int mExtraRandomHeight;
};

#endif
