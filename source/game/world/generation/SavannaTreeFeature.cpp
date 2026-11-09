#include "game/world/generation/SavannaTreeFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/NewLeafBlock.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include <cstdlib>

static const BlockState* savannaTrunkState = NULL;
static const BlockState* savannaLeafState = NULL;

SavannaTreeFeature::SavannaTreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 5, savannaTrunkState, savannaLeafState, false)
{
}

SavannaTreeFeature::~SavannaTreeFeature()
{
}

void SavannaTreeFeature::staticCtor()
{
    savannaTrunkState = Blocks::log2->defaultBlockState()->setValue(NewLogBlock::VARIANT, PlanksBlock::Variant::ACACIA);
    savannaLeafState = Blocks::leaves2->defaultBlockState()
                           ->setValue(NewLeafBlock::VARIANT, PlanksBlock::Variant::ACACIA)
                           ->setValue(LeafBlock::CHECK_DECAY, false);
}

void SavannaTreeFeature::placeLogAt(Level* level, const BlockPos& pos)
{
    setBlock(level, pos, mTrunkState, Block::UPDATE_NONE);
}

void SavannaTreeFeature::placeLeafAt(Level* level, const BlockPos& pos)
{
    Material* material = level->getBlockState(pos)->getMaterial();
    if (material == Material::AIR || material == Material::LEAVES)
    {
        setBlock(level, pos, mLeafState, Block::UPDATE_NONE);
    }
}

bool SavannaTreeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "SavannaTreeFeature::place");
    int height = random.nextInt(3) + random.nextInt(3) + mBaseHeight;
    bool free = true;
    if (origin.y < 1 || origin.y + height + 1 > 256)
    {
        PIXEndNamedEvent();
        return false;
    }

    for (int y = origin.y; y <= origin.y + height + 1; ++y)
    {
        int radius = 1;
        if (y == origin.y)
            radius = 0;
        if (y >= origin.y + height - 1)
            radius = 2;
        for (int x = origin.x - radius; x <= origin.x + radius && free; ++x)
        {
            for (int z = origin.z - radius; z <= origin.z + radius && free; ++z)
            {
                if (y >= 0 && y < 256)
                {
                    if (!isFree(level->getBlockState(BlockPos(x, y, z))->getBlock()))
                        free = false;
                }
                else
                {
                    free = false;
                }
            }
        }
    }
    if (!free)
    {
        PIXEndNamedEvent();
        return false;
    }

    Block* ground = level->getBlockState(origin.below())->getBlock();
    if ((ground != Blocks::grass && ground != Blocks::dirt) || origin.y >= 255 - height)
    {
        PIXEndNamedEvent();
        return false;
    }
    else
    {
        setDirtAt(level, origin.below());

        const Direction* trunkDirection = Direction::Plane::HORIZONTAL->getRandomDirection(random);
        int bendStart = height - random.nextInt(4) - 1;
        int bendLength = 3 - random.nextInt(3);
        int x = origin.x;
        int z = origin.z;
        int topY = 0;
        MutableBlockPos trunkPos(0, 0, 0);
        for (int y = 0; y < height; ++y)
        {
            int trunkY = origin.y + y;
            if (y >= bendStart && bendLength > 0)
            {
                x += trunkDirection->getStepX();
                z += trunkDirection->getStepZ();
                --bendLength;
            }
            trunkPos.set(x, trunkY, z);
            Material* material = level->getBlockState(trunkPos)->getMaterial();
            if (material == Material::AIR || material == Material::LEAVES)
            {
                placeLogAt(level, trunkPos);
                topY = trunkY;
            }
        }

        BlockPos canopy(x, topY, z);
        for (int dx = -3; dx <= 3; ++dx)
        {
            for (int dz = -3; dz <= 3; ++dz)
            {
                if (abs(dx) != 3 || abs(dz) != 3)
                    placeLeafAt(level, canopy.offset(dx, 0, dz));
            }
        }
        canopy = canopy.above();
        for (int dx = -1; dx <= 1; ++dx)
        {
            for (int dz = -1; dz <= 1; ++dz)
                placeLeafAt(level, canopy.offset(dx, 0, dz));
        }
        placeLeafAt(level, canopy.east(2));
        placeLeafAt(level, canopy.west(2));
        placeLeafAt(level, canopy.south(2));
        placeLeafAt(level, canopy.north(2));

        x = origin.x;
        z = origin.z;
        const Direction* branchDirection = Direction::Plane::HORIZONTAL->getRandomDirection(random);
        if (branchDirection != trunkDirection)
        {
            int branchStart = bendStart - random.nextInt(2) - 1;
            int branchLength = random.nextInt(3) + 1;
            topY = 0;
            for (int branchY = branchStart; branchY < height && branchLength > 0; --branchLength)
            {
                if (branchY >= 1)
                {
                    int y = origin.y + branchY;
                    x += branchDirection->getStepX();
                    z += branchDirection->getStepZ();
                    const BlockPos& branchPos = BlockPos(x, y, z);
                    Material* material = level->getBlockState(branchPos)->getMaterial();
                    if (material == Material::AIR || material == Material::LEAVES)
                    {
                        placeLogAt(level, branchPos);
                        topY = y;
                    }
                }
                ++branchY;
            }

            if (topY > 0)
            {
                BlockPos branchCanopy(x, topY, z);
                for (int dx = -2; dx <= 2; ++dx)
                {
                    for (int dz = -2; dz <= 2; ++dz)
                    {
                        if (abs(dx) != 2 || abs(dz) != 2)
                            placeLeafAt(level, branchCanopy.offset(dx, 0, dz));
                    }
                }
                branchCanopy = branchCanopy.above();
                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dz = -1; dz <= 1; ++dz)
                        placeLeafAt(level, branchCanopy.offset(dx, 0, dz));
                }
            }
        }

        PIXEndNamedEvent();
        return true;
    }
}
