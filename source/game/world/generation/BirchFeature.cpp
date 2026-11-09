#include "game/world/generation/BirchFeature.hpp"

#include "game/util/Random.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include <cstdlib>

static const BlockState* birchTrunkState = NULL;
static const BlockState* birchLeafState = NULL;

BirchFeature::BirchFeature(bool doBlockNotify, bool tallBirch)
    : AbstractTreeFeature(doBlockNotify, 5, birchTrunkState, birchLeafState, false),
      mTallBirch(tallBirch)
{
}

void BirchFeature::staticCtor()
{
    birchTrunkState = Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::BIRCH);
    birchLeafState = Blocks::leaves->defaultBlockState()
                         ->setValue(OldLeafBlock::VARIANT, PlanksBlock::Variant::BIRCH)
                         ->setValue(LeafBlock::CHECK_DECAY, false);
}

bool BirchFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "BirchFeature::place");
    int height = random.nextInt(3) + mBaseHeight;
    if (mTallBirch)
    {
        height += random.nextInt(7);
    }

    bool free = true;
    if (origin.y < 1 || origin.y + height + 1 > 256)
    {
        PIXEndNamedEvent();
        return false;
    }

    MutableBlockPos check(0, 0, 0);
    for (int y = origin.y; y <= origin.y + height + 1; ++y)
    {
        int radius = 1;
        if (y == origin.y)
        {
            radius = 0;
        }
        if (y >= origin.y + height - 1)
        {
            radius = 2;
        }
        for (int x = origin.x - radius; x <= origin.x + radius && free; ++x)
        {
            for (int z = origin.z - radius; z <= origin.z + radius && free; ++z)
            {
                if (y >= 0 && y < 256)
                {
                    check.set(x, y, z);
                    if (!isFree(level->getBlockState(check)->getBlock()))
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
    if ((ground != Blocks::grass && ground != Blocks::dirt && ground != Blocks::farmland) || origin.y >= 255 - height)
    {
        PIXEndNamedEvent();
        return false;
    }

    if (gApp.getLevelGenerationOptions() != NULL)
    {
        LevelGenerationOptions* levelGenerationOptions = gApp.getLevelGenerationOptions();
        if (levelGenerationOptions->checkIntersects(origin.offset(-3, -1, -3), origin.offset(3, height, 3)))
        {
            PIXEndNamedEvent();
            return false;
        }
    }

    setDirtAt(level, origin.below());
    if (!placeTrunk(level, origin, random, height))
        return true;

    MutableBlockPos leaves(0, 0, 0);
    for (int y = origin.y - 3 + height; y <= origin.y + height; ++y)
    {
        int yOffset = y - (origin.y + height);
        int radius = 1 - yOffset / 2;
        for (int x = origin.x - radius; x <= origin.x + radius; ++x)
        {
            int xOffset = x - origin.x;
            for (int z = origin.z - radius; z <= origin.z + radius; ++z)
            {
                if (std::abs(xOffset) != radius || std::abs(z - origin.z) != radius
                    || (random.nextInt(2) != 0 && yOffset != 0))
                {
                    leaves.set(x, y, z);
                    Material* material = level->getBlockState(leaves)->getMaterial();
                    if (material == Material::AIR || material == Material::LEAVES)
                    {
                        setBlock(level, leaves, mLeafState, Block::UPDATE_NONE);
                    }
                }
            }
        }
    }
    PIXEndNamedEvent();
    return true;
}
