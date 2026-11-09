#ifndef GAME_WORLD_GENERATION_BIGTREEFEATURE_HPP
#define GAME_WORLD_GENERATION_BIGTREEFEATURE_HPP

#include <vector>

#include "game/world/generation/AbstractTreeFeature.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/util/Random.hpp"

class Block;
class BlockState;
class Level;
class Random;

class BigTreeFeature : public AbstractTreeFeature
{
public:
    class FoliageCoords : public BlockPos
    {
    public:
        FoliageCoords(const BlockPos& pos, int branchBase);
        int getBranchBase();

    private:
        int mBranchBase;
    };

    BigTreeFeature(bool doBlockNotify);
    virtual ~BigTreeFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);
    virtual void init();
    float treeShape(int height);
    int getSteps(const BlockPos& pos);
    int checkLine(const BlockPos& from, const BlockPos& to);
    void prepare();
    void crossection(const BlockPos& pos, float radius, const BlockState* state);
    float foliageShape(int layer);
    void foliageCluster(const BlockPos& pos);
    LogBlock::LogAxis* getLogAxis(const BlockPos& from, const BlockPos& to);
    void limb(const BlockPos& from, const BlockPos& to, Block* block);
    void makeFoliage();
    bool trimBranches(int branchBase);
    void makeTrunk();
    void makeBranches();
    bool checkLocation();
    int getPrimaryDirection(const BlockPos& pos);

private:
    Random mRandom;
    Level* mLevel;
    BlockPos mBasePos;
    int mHeightLimit;
    int mHeight;
    double mHeightAttenuation;
    double mBranchSlope;
    double mScaleWidth;
    double mLeafDensity;
    int mTrunkSize;
    int mHeightLimitLimit;
    int mLeafDistanceLimit;
    std::vector<FoliageCoords> mFoliageCoords;
};

#endif
