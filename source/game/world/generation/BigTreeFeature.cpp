#include "game/world/generation/BigTreeFeature.hpp"

#include <cmath>
#include <cstdlib>

#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

BigTreeFeature::FoliageCoords::FoliageCoords(const BlockPos& pos, int branchBase)
    : BlockPos(pos.x, pos.y, pos.z),
      mBranchBase(branchBase)
{
}

int BigTreeFeature::FoliageCoords::getBranchBase()
{
    return mBranchBase;
}

BigTreeFeature::BigTreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 5, Blocks::log->defaultBlockState(), Blocks::leaves->defaultBlockState(),
                          false),
      mRandom(),
      mBasePos(),
      mFoliageCoords()
{
    mHeightLimit = 0;
    mHeight = 0;
    mBasePos = BlockPos::ZERO;
    mHeightAttenuation = 0.618;
    mBranchSlope = 0.381;
    mScaleWidth = 1.0;
    mLeafDensity = 1.0;
    mTrunkSize = 1;
    mHeightLimitLimit = 12;
    mLeafDistanceLimit = 4;
}

BigTreeFeature::~BigTreeFeature()
{
}

float BigTreeFeature::treeShape(int height)
{
    if ((float)height < (float)mHeightLimit * 0.3f)
        return -1.0f;

    float half = (float)mHeightLimit * 0.5f;
    float offset = half - (float)height;
    float width = sqrtf(half * half - offset * offset);
    float shape = 0.0f;
    if (offset == 0.0f)
        shape = half * 0.5f;
    else if (std::fabs(offset) < half)
        shape = width * 0.5f;
    return shape;
}

int BigTreeFeature::getSteps(const BlockPos& pos)
{
    int x = abs(pos.x);
    int y = abs(pos.y);
    int z = abs(pos.z);
    if (z > x && z > y)
        return z;
    if (y > x)
        return y;
    return x;
}

int BigTreeFeature::checkLine(const BlockPos& from, const BlockPos& to)
{
    BlockPos delta = to.offset(-from.x, -from.y, -from.z);
    int steps = getSteps(delta);
    float stepCount = (float)steps;
    float dx = (float)delta.x / stepCount;
    float dy = (float)delta.y / stepCount;
    float dz = (float)delta.z / stepCount;
    if (steps == 0)
        return -1;

    for (int i = 0; i <= steps; ++i)
    {
        float step = (float)i;
        BlockPos pos = from.offset((double)(step * dx + 0.5f), (double)(step * dy + 0.5f), (double)(step * dz + 0.5f));
        if (!isFree(mLevel->getBlockState(pos)->getBlock()))
            return i;
    }
    return -1;
}

void BigTreeFeature::prepare()
{
    int height = (int)((double)mHeightLimit * mHeightAttenuation);
    if (height >= mHeightLimit)
        height = mHeightLimit - 1;
    mHeight = height;

    int clustersPerLayer = (int)(1.382 + pow(mLeafDensity * (double)mHeightLimit / 13.0, 2));
    if (clustersPerLayer < 1)
        clustersPerLayer = 1;

    int trunkTop = mBasePos.y + mHeight;
    int layer = mHeightLimit - mLeafDistanceLimit;
    mFoliageCoords.clear();
    mFoliageCoords.push_back(FoliageCoords(mBasePos.above(layer), trunkTop));

    for (; layer >= 0; --layer)
    {
        float shape = treeShape(layer);
        if (shape < 0.0f)
            continue;

        for (int i = 0; i < clustersPerLayer; ++i)
        {
            double distance = mScaleWidth * shape * ((double)mRandom.nextFloat() + 0.328);
            double angle = mRandom.nextFloat() * 2.0f * 3.1415927f;
            BlockPos cluster = mBasePos.offset(distance * (double)Mth::sin(angle) + 0.5, (double)layer - 1.0,
                                               distance * (double)Mth::cos(angle) + 0.5);
            if (checkLine(cluster, cluster.above(mLeafDistanceLimit)) != -1)
                continue;

            double branchY = (double)cluster.y
                             - (double)sqrtf((float)((mBasePos.x - cluster.x) * (mBasePos.x - cluster.x)
                                                     + (mBasePos.z - cluster.z) * (mBasePos.z - cluster.z)))
                                   * mBranchSlope;
            const BlockPos& branchBase = BlockPos(mBasePos.x, branchY > (double)trunkTop ? trunkTop : (int)branchY,
                                                  mBasePos.z);
            if (checkLine(branchBase, cluster) == -1)
                mFoliageCoords.push_back(FoliageCoords(cluster, branchBase.y));
        }
    }
}

void BigTreeFeature::crossection(const BlockPos& pos, float radius, const BlockState* state)
{
    int range = (int)((double)radius + 0.618);
    double radiusSquared = (double)(radius * radius);
    for (int x = -range; x <= range; ++x)
    {
        for (int z = -range; z <= range; ++z)
        {
            double xDistance = (double)abs(x) + 0.5;
            double zDistance = (double)abs(z) + 0.5;
            if (pow(xDistance, 2) + pow(zDistance, 2) <= radiusSquared)
            {
                BlockPos target = pos.offset(x, 0, z);
                Material* material = mLevel->getBlockState(target)->getMaterial();
                if (material == Material::AIR || material == Material::LEAVES)
                    setBlock(mLevel, target, state, Block::UPDATE_NONE);
            }
        }
    }
}

float BigTreeFeature::foliageShape(int layer)
{
    if (layer < 0 || layer >= mLeafDistanceLimit)
        return -1.0f;
    if (layer == 0 || layer == mLeafDistanceLimit - 1)
        return 2.0f;
    return 3.0f;
}

void BigTreeFeature::foliageCluster(const BlockPos& pos)
{
    for (int layer = 0; layer < mLeafDistanceLimit; ++layer)
    {
        BlockPos target = pos.above(layer);
        const BlockState* leaves = Blocks::leaves->defaultBlockState()->setValue(LeafBlock::CHECK_DECAY, false);
        crossection(target, foliageShape(layer), leaves);
    }
}

LogBlock::LogAxis* BigTreeFeature::getLogAxis(const BlockPos& from, const BlockPos& to)
{
    int dx = abs(to.x - from.x);
    int dz = abs(to.z - from.z);
    int largest = dx < dz ? dz : dx;
    LogBlock::LogAxis* axis = LogBlock::Y;
    if (largest > 0)
    {
        if (dx == largest)
            axis = LogBlock::X;
        else if (dz == largest)
            axis = LogBlock::Z;
    }
    return axis;
}

void BigTreeFeature::limb(const BlockPos& from, const BlockPos& to, Block* block)
{
    BlockPos delta = to.offset(-from.x, -from.y, -from.z);
    int steps = getSteps(delta);
    float stepCount = (float)steps;
    float dx = (float)delta.x / stepCount;
    float dy = (float)delta.y / stepCount;
    float dz = (float)delta.z / stepCount;
    for (int i = 0; i <= steps; ++i)
    {
        float step = (float)i;
        BlockPos pos = from.offset((double)(step * dx + 0.5f), (double)(step * dy + 0.5f), (double)(step * dz + 0.5f));
        LogBlock::LogAxis* axis = getLogAxis(from, pos);
        setBlock(mLevel, pos, block->defaultBlockState()->setValue(LogBlock::AXIS, axis), Block::UPDATE_NONE);
    }
}

void BigTreeFeature::makeFoliage()
{
    for (std::vector<FoliageCoords>::iterator it = mFoliageCoords.begin(); it != mFoliageCoords.end(); ++it)
        foliageCluster(*it);
}

bool BigTreeFeature::trimBranches(int branchBase)
{
    return (double)branchBase >= (double)mHeightLimit * 0.2;
}

void BigTreeFeature::makeTrunk()
{
    const BlockPos& base = mBasePos;
    BlockPos top = base.above(mHeight);
    Block* log = Blocks::log;
    limb(base, top, log);
    if (mTrunkSize == 2)
    {
        limb(base.east(), top.east(), log);
        limb(base.east().south(), top.east().south(), log);
        limb(base.south(), top.south(), log);
    }
}

void BigTreeFeature::makeBranches()
{
    for (std::vector<FoliageCoords>::iterator it = mFoliageCoords.begin(); it != mFoliageCoords.end(); ++it)
    {
        int branchBaseY = it->getBranchBase();
        const BlockPos& branchBase = BlockPos(mBasePos.x, branchBaseY, mBasePos.z);
        if (branchBase != *it && trimBranches(branchBaseY - mBasePos.y))
            limb(branchBase, *it, Blocks::log);
    }
}

void BigTreeFeature::init()
{
    mLeafDistanceLimit = 5;
}

bool BigTreeFeature::checkLocation()
{
    Block* ground = mLevel->getBlockState(mBasePos.below())->getBlock();
    if (ground != Blocks::dirt && ground != Blocks::grass && ground != Blocks::farmland)
        return false;

    int obstruction = checkLine(mBasePos, mBasePos.above(mHeightLimit - 1));
    if (obstruction == -1)
        return true;
    if (obstruction < 6)
        return false;
    mHeightLimit = obstruction;
    return true;
}

bool BigTreeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "BigTreeFeature::place");
    mLevel = level;
    mBasePos = origin;
    mRandom = Random(random.nextLong());
    if (mHeightLimit == 0)
        mHeightLimit = random.nextInt(mHeightLimitLimit) + 5;

    if (!checkLocation())
    {
        PIXEndNamedEvent();
        return false;
    }

    prepare();
    makeFoliage();
    makeTrunk();
    makeBranches();
    PIXEndNamedEvent();
    return true;
}
