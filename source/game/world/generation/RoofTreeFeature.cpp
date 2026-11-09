#include "game/world/generation/RoofTreeFeature.hpp"

#include <cstdlib>

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

static const BlockState* roofTreeTrunkState = NULL;
static const BlockState* roofTreeLeafState = NULL;

RoofTreeFeature::RoofTreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 6, roofTreeTrunkState, roofTreeLeafState, false)
{
}

RoofTreeFeature::~RoofTreeFeature()
{
}

void RoofTreeFeature::staticCtor()
{
    roofTreeTrunkState = Blocks::log2->defaultBlockState()->setValue(NewLogBlock::VARIANT,
                                                                     PlanksBlock::Variant::DARK_OAK);
    roofTreeLeafState = Blocks::leaves2->defaultBlockState()
                            ->setValue(NewLeafBlock::VARIANT, PlanksBlock::Variant::DARK_OAK)
                            ->setValue(LeafBlock::CHECK_DECAY, false);
}

bool RoofTreeFeature::canPlaceTreeOfHeight(Level* level, const BlockPos& pos, int height)
{
    const int baseX = pos.x;
    const int baseY = pos.y;
    const int baseZ = pos.z;
    BlockPos current;
    for (int y = 0; y <= height + 1; ++y)
    {
        int radius = 1;
        if (y == 0)
        {
            radius = 0;
        }
        if (y >= height - 1)
        {
            radius = 2;
        }
        for (int x = -radius; x <= radius; ++x)
        {
            for (int z = -radius; z <= radius; ++z)
            {
                current = BlockPos(baseX + x, baseY + y, baseZ + z);
                if (!isFree(level->getBlockState(current)->getBlock()))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

void RoofTreeFeature::placeLogAt(Level* level, const BlockPos& pos)
{
    if (isFree(level->getBlockState(pos)->getBlock()))
    {
        setBlock(level, pos, mTrunkState, Block::UPDATE_NONE);
    }
}

void RoofTreeFeature::placeLeafAt(Level* level, int x, int y, int z)
{
    BlockPos leafPos(x, y, z);
    if (level->getBlockState(leafPos)->getMaterial() == Material::AIR)
    {
        setBlock(level, leafPos, mLeafState, Block::UPDATE_NONE);
    }
}

bool RoofTreeFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "RoofTreeFeature::place");

    const int heightOffset = random.nextInt(3);
    const int height = heightOffset + random.nextInt(2) + mBaseHeight;
    const int baseX = pos.x;
    const int baseZ = pos.z;
    const int treeTop = pos.y + height;
    if (pos.y < 1 || treeTop + 1 > 256)
    {
        PIXEndNamedEvent();
        return false;
    }

    const BlockPos ground = pos.below();
    Block* groundBlock = level->getBlockState(ground)->getBlock();
    if ((groundBlock != Blocks::grass && groundBlock != Blocks::dirt) || pos.y >= 255 - height)
    {
        PIXEndNamedEvent();
        return false;
    }

    if (!canPlaceTreeOfHeight(level, pos, height))
    {
        PIXEndNamedEvent();
        return false;
    }

    setDirtAt(level, ground);
    setDirtAt(level, ground.east());
    setDirtAt(level, ground.south());
    setDirtAt(level, ground.south().east());

    const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(random);
    int bendStart = height - random.nextInt(4);
    int bendLeft = 2 - random.nextInt(3);
    int x = baseX;
    int z = baseZ;
    int top = treeTop - 1;

    MutableBlockPos cursor(0, 0, 0);
    for (int y = 0; y < height; ++y)
    {
        const int logY = pos.y + y;
        if (y >= bendStart && bendLeft > 0)
        {
            x += direction->getStepX();
            z += direction->getStepZ();
            --bendLeft;
        }

        cursor.set(x, logY, z);
        Material* material = level->getBlockState(cursor)->getMaterial();
        if (material == Material::AIR || material == Material::LEAVES)
        {
            placeLogAt(level, cursor);
            placeLogAt(level, cursor.east());
            placeLogAt(level, cursor.south());
            placeLogAt(level, cursor.east().south());
            top = logY;
        }
    }

    for (int dx = -2; dx <= 0; ++dx)
    {
        for (int dz = -2; dz <= 0; ++dz)
        {
            int dy = -1;
            placeLeafAt(level, x + dx, top + dy, z + dz);
            placeLeafAt(level, 1 + x - dx, top + dy, z + dz);
            placeLeafAt(level, x + dx, top + dy, 1 + z - dz);
            placeLeafAt(level, 1 + x - dx, top + dy, 1 + z - dz);
            if ((dx > -2 || dz > -1) && (dx != -1 || dz != -2))
            {
                dy = 1;
                placeLeafAt(level, x + dx, top + dy, z + dz);
                placeLeafAt(level, 1 + x - dx, top + dy, z + dz);
                placeLeafAt(level, x + dx, top + dy, 1 + z - dz);
                placeLeafAt(level, 1 + x - dx, top + dy, 1 + z - dz);
            }
        }
    }

    if (random.nextBoolean())
    {
        placeLeafAt(level, x, top + 2, z);
        placeLeafAt(level, x + 1, top + 2, z);
        placeLeafAt(level, x + 1, top + 2, z + 1);
        placeLeafAt(level, x, top + 2, z + 1);
    }

    for (int dx = -3; dx <= 4; ++dx)
    {
        for (int dz = -3; dz <= 4; ++dz)
        {
            if ((dx != -3 || dz != -3) && (dx != -3 || dz != 4) && (dx != 4 || dz != -3) && (dx != 4 || dz != 4)
                && (std::abs(dx) < 3 || std::abs(dz) < 3))
            {
                placeLeafAt(level, x + dx, top, z + dz);
            }
        }
    }

    for (int branchX = -1; branchX <= 2; ++branchX)
    {
        for (int branchZ = -1; branchZ <= 2; ++branchZ)
        {
            if ((branchX < 0 || branchX > 1 || branchZ < 0 || branchZ > 1) && random.nextInt(3) <= 0)
            {
                const int branchLength = random.nextInt(3) + 2;
                for (int i = 0; i < branchLength; ++i)
                {
                    placeLogAt(level, BlockPos(baseX + branchX, top - i - 1, baseZ + branchZ));
                }

                for (int dx = -1; dx <= 1; ++dx)
                {
                    for (int dz = -1; dz <= 1; ++dz)
                    {
                        placeLeafAt(level, x + branchX + dx, top, z + branchZ + dz);
                    }
                }

                for (int dx = -2; dx <= 2; ++dx)
                {
                    for (int dz = -2; dz <= 2; ++dz)
                    {
                        if (std::abs(dx) != 2 || std::abs(dz) != 2)
                        {
                            placeLeafAt(level, x + branchX + dx, top - 1, z + branchZ + dz);
                        }
                    }
                }
            }
        }
    }

    PIXEndNamedEvent();
    return true;
}
