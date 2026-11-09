#include "game/world/generation/MegaPineTreeFeature.hpp"

#include <cstdlib>

#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const BlockState* megaPineTrunkState = NULL;
static const BlockState* megaPineLeafState = NULL;
static const BlockState* megaPinePodzolState = NULL;

void MegaPineTreeFeature::staticCtor()
{
    megaPineTrunkState = Blocks::log->defaultBlockState()->setValue<PlanksBlock::Variant*>(
        OldLogBlock::VARIANT, PlanksBlock::Variant::SPRUCE);
    megaPineLeafState = Blocks::leaves->defaultBlockState()->setValue<PlanksBlock::Variant*>(
        OldLeafBlock::VARIANT, PlanksBlock::Variant::SPRUCE);
    megaPinePodzolState = Blocks::dirt->defaultBlockState()->setValue<DirtBlock::Variant*>(DirtBlock::VARIANT,
                                                                                           DirtBlock::Variant::PODZOL);
}

MegaPineTreeFeature::MegaPineTreeFeature(bool doBlockNotify, bool useHeight)
    : MegaTreeFeature(doBlockNotify, 13, 15, megaPineTrunkState, megaPineLeafState),
      mUseHeight(useHeight)
{
}

MegaPineTreeFeature::~MegaPineTreeFeature()
{
}

void MegaPineTreeFeature::createCrown(Level* level, int x, int z, int y, int radius, Random& random)
{
    int crownHeight = random.nextInt(5) + (mUseHeight ? mBaseHeight : 3);
    int previousRadius = 0;

    for (int layer = y - crownHeight; layer <= y; ++layer)
    {
        int distance = y - layer;
        int layerRadius = radius + Mth::floor((float)distance / (float)crownHeight * 3.5f);
        placeDoubleTrunkLeaves(level, BlockPos(x, layer, z),
                               layerRadius
                                   + (distance > 0 && layerRadius == previousRadius && (layer & 1) == 0 ? 1 : 0));
        previousRadius = layerRadius;
    }
}

bool MegaPineTreeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    int height = calcTreeHeigth(random);
    if (!prepareTree(level, random, origin, height))
    {
        return false;
    }

    createCrown(level, origin.x, origin.z, origin.y + height, 0, random);

    for (int y = 0; y < height; ++y)
    {
        const BlockState* state = level->getBlockState(origin.above(y));
        if (state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES)
        {
            setBlock(level, origin.above(y), mTrunkState, Block::UPDATE_NONE);
        }

        if (y < height - 1)
        {
            state = level->getBlockState(origin.offset(1, y, 0));
            if (state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES)
            {
                setBlock(level, origin.offset(1, y, 0), mTrunkState, Block::UPDATE_NONE);
            }

            state = level->getBlockState(origin.offset(1, y, 1));
            if (state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES)
            {
                setBlock(level, origin.offset(1, y, 1), mTrunkState, Block::UPDATE_NONE);
            }

            state = level->getBlockState(origin.offset(0, y, 1));
            if (state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES)
            {
                setBlock(level, origin.offset(0, y, 1), mTrunkState, Block::UPDATE_NONE);
            }
        }
    }

    return true;
}

void MegaPineTreeFeature::placePodzolAt(Level* level, const BlockPos& pos)
{
    for (int y = 2; y >= -3; --y)
    {
        BlockPos checkPos = pos.above(y);
        const BlockState* state = level->getBlockState(checkPos);
        Block* block = state->getBlock();
        if (block == Blocks::grass || block == Blocks::dirt)
        {
            setBlock(level, checkPos, megaPinePodzolState, Block::UPDATE_NONE);
            return;
        }

        if (state->getMaterial() != Material::AIR && y < 0)
        {
            return;
        }
    }
}

void MegaPineTreeFeature::placePodzolCircle(Level* level, const BlockPos& pos)
{
    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            if (std::abs(x) != 2 || std::abs(z) != 2)
            {
                placePodzolAt(level, pos.offset(x, 0, z));
            }
        }
    }
}

void MegaPineTreeFeature::postPlaceTree(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "MegaPineTreeFeature::postPlaceTree");

    placePodzolCircle(level, pos.west().north());
    placePodzolCircle(level, pos.east(2).north());
    placePodzolCircle(level, pos.west().south(2));
    placePodzolCircle(level, pos.east(2).south(2));

    for (int i = 0; i < 5; ++i)
    {
        int value = random.nextInt(64);
        int x = value % 8;
        int z = value / 8;
        if (x == 0 || x == 7 || z == 0 || z == 7)
        {
            placePodzolCircle(level, pos.offset(x - 3, 0, z - 3));
        }
    }

    PIXEndNamedEvent();
}
