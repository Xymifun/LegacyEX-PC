#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/generation/SwampTreeFeature.hpp"

#include <cstdlib>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/VineBlock.hpp"


static const BlockState* swampTrunkState = NULL;
static const BlockState* swampLeafState = NULL;

SwampTreeFeature::SwampTreeFeature() : AbstractTreeFeature(false, 5, swampTrunkState, swampLeafState, false)
{
}

void SwampTreeFeature::staticCtor()
{
    swampTrunkState = Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::OAK);
    swampLeafState = Blocks::leaves->defaultBlockState()
                         ->setValue(OldLeafBlock::VARIANT, PlanksBlock::Variant::OAK)
                         ->setValue(LeafBlock::CHECK_DECAY, false);
}

void SwampTreeFeature::addVine(Level* level, const BlockPos& pos, const BooleanProperty* property)
{
    const BlockState* state = Blocks::vine->defaultBlockState()->setValue(property, true);
    BlockPos vinePos = pos;
    setBlock(level, vinePos, state, Block::UPDATE_NONE);
    vinePos = vinePos.below();
    for (int length = 4; level->getBlockState(vinePos)->getBlock()->mMaterial == Material::AIR && length > 0; --length)
    {
        setBlock(level, vinePos, state, Block::UPDATE_NONE);
        vinePos = vinePos.below();
    }
}

bool SwampTreeFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "SwampTreeFeature::place");
    BlockPos origin = pos;
    int height = random.nextInt(4) + mBaseHeight;
    while (level->getBlockState(origin.below())->getMaterial() == Material::WATER)
    {
        origin = origin.below();
    }

    bool free = true;

    if (origin.y < 1 || origin.y + height + 1 > 128)
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

    MutableBlockPos checkPos(0, 0, 0);
    for (int y = origin.y; y <= origin.y + height + 1; ++y)
    {
        int radius = 1;
        if (y == origin.y)
            radius = 0;
        if (y >= origin.y + height - 1)
            radius = 3;
        for (int x = origin.x - radius; x <= origin.x + radius && free; ++x)
        {
            for (int z = origin.z - radius; z <= origin.z + radius && free; ++z)
            {
                if (y >= 0 && y < 128)
                {
                    checkPos.set(x, y, z);
                    const BlockState* state = level->getBlockState(checkPos);
                    Block* block = state->getBlock();
                    if (state->getMaterial() != Material::AIR && state->getMaterial() != Material::LEAVES)
                    {
                        if ((block != Blocks::water && block != Blocks::flowing_water) || y > origin.y)
                            free = false;
                    }
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
    if ((ground != Blocks::grass && ground != Blocks::dirt) || origin.y >= 127 - height)
    {
        PIXEndNamedEvent();
        return false;
    }

    setDirtAt(level, origin.below());

    MutableBlockPos leafPos(0, 0, 0);
    for (int y = origin.y - 3 + height; y <= origin.y + height; ++y)
    {
        int yOffset = y - (origin.y + height);
        int radius = 2 - yOffset / 2;
        for (int x = origin.x - radius; x <= origin.x + radius; ++x)
        {
            int xOffset = x - origin.x;
            for (int z = origin.z - radius; z <= origin.z + radius; ++z)
            {
                int zOffset = z - origin.z;
                if (!(abs(xOffset) == radius && abs(zOffset) == radius) || (random.nextInt(2) != 0 && yOffset != 0))
                {
                    leafPos.set(x, y, z);
                    if (!level->getBlockState(leafPos)->isSolid())
                        setBlock(level, leafPos, mLeafState, Block::UPDATE_NONE);
                }
            }
        }
    }

    for (int y = 0; y < height; ++y)
    {
        const BlockState* state = level->getBlockState(origin.above(y));
        Block* block = state->getBlock();
        if (state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES
            || block == Blocks::flowing_water || block == Blocks::water)
            setBlock(level, origin.above(y), mTrunkState, Block::UPDATE_NONE);
    }

    MutableBlockPos vineSource(0, 0, 0);
    for (int y = origin.y - 3 + height; y <= origin.y + height; ++y)
    {
        int yOffset = y - (origin.y + height);
        int radius = 2 - yOffset / 2;
        for (int x = origin.x - radius; x <= origin.x + radius; ++x)
        {
            for (int z = origin.z - radius; z <= origin.z + radius; ++z)
            {
                vineSource.set(x, y, z);
                if (level->getBlockState(vineSource)->getMaterial() == Material::LEAVES)
                {
                    BlockPos west = vineSource.west();
                    BlockPos east = vineSource.east();
                    BlockPos north = vineSource.north();
                    BlockPos south = vineSource.south();
                    if (random.nextInt(4) == 0 && level->getBlockState(west)->getMaterial() == Material::AIR)
                        addVine(level, west, VineBlock::EAST);
                    if (random.nextInt(4) == 0 && level->getBlockState(east)->getMaterial() == Material::AIR)
                        addVine(level, east, VineBlock::WEST);
                    if (random.nextInt(4) == 0 && level->getBlockState(north)->getMaterial() == Material::AIR)
                        addVine(level, north, VineBlock::SOUTH);
                    if (random.nextInt(4) == 0 && level->getBlockState(south)->getMaterial() == Material::AIR)
                        addVine(level, south, VineBlock::NORTH);
                }
            }
        }
    }

    PIXEndNamedEvent();
    return true;
}
