#include "game/world/generation/TreeFeature.hpp"

#include <cstdlib>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/CocoaBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/VineBlock.hpp"
#include "game/core/PIXNamedEvents.hpp"


static const BlockState* treeTrunkState = NULL;
static const BlockState* treeLeafState = NULL;

TreeFeature::TreeFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 4, treeTrunkState, treeLeafState, false)
{
}

TreeFeature::TreeFeature(bool doBlockNotify, int baseHeight, const BlockState* trunkState, const BlockState* leafState,
                         bool hasVines)
    : AbstractTreeFeature(doBlockNotify, baseHeight, trunkState, leafState, hasVines)
{
}

void TreeFeature::staticCtor()
{
    treeTrunkState = Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::OAK);
    treeLeafState = Blocks::leaves->defaultBlockState()
                        ->setValue(OldLeafBlock::VARIANT, PlanksBlock::Variant::OAK)
                        ->setValue(LeafBlock::CHECK_DECAY, false);
}

int TreeFeature::prepareSpawn(Level* level, const BlockPos& pos, int height)
{
    PIXBeginNamedEvent(0.0f, "TreeFeature::prepareSpawn");
    int free = 1;
    if (pos.y < 1 || pos.y + height + 1 > 256)
    {
        PIXEndNamedEvent();
        return 0;
    }

    if (gApp.getLevelGenerationOptions() != NULL)
    {
        PIXBeginNamedEvent(0.0f, "TreeFeature checking intersects");
        LevelGenerationOptions* levelGenerationOptions = gApp.getLevelGenerationOptions();
        bool intersects = levelGenerationOptions->checkIntersects(pos.offset(-2, -1, -2), pos.offset(2, height, 2));
        PIXEndNamedEvent();
        if (intersects)
        {
            PIXEndNamedEvent();
            return 0;
        }
    }

    BlockPos check(0, 0, 0);
    for (int y = pos.y; y <= pos.y + height + 1; ++y)
    {
        int radius = 1;
        if (y == pos.y)
            radius = 0;
        if (y >= pos.y + height - 1)
            radius = 2;
        for (int x = pos.x - radius; x <= pos.x + radius && free; ++x)
        {
            for (int z = pos.z - radius; z <= pos.z + radius && free; ++z)
            {
                if (y >= 0 && y < 256)
                {
                    check.x = x;
                    check.y = y;
                    check.z = z;
                    if (!isFree(level->getBlockState(check)->getBlock()))
                        free = 0;
                }
                else
                {
                    free = 0;
                }
            }
        }
    }

    if (!free)
    {
        PIXEndNamedEvent();
        return 0;
    }

    Block* ground = level->getBlockState(pos.below())->getBlock();
    if ((ground != Blocks::grass && ground != Blocks::dirt && ground != Blocks::farmland) || pos.y >= 255 - height)
    {
        PIXEndNamedEvent();
        return 0;
    }

    setDirtAt(level, pos.below());
    PIXEndNamedEvent();
    return free;
}

void TreeFeature::placeLeaf(Level* level, const MutableBlockPos& pos)
{
    Material* material = level->getBlockState(pos)->getMaterial();
    if (material == Material::AIR || material == Material::LEAVES || material == Material::REPLACEABLE_PLANT)
        setBlock(level, pos, mLeafState, Block::UPDATE_NONE);
}

void TreeFeature::addHangingVine(Level* level, const BlockPos& pos, const BooleanProperty* property)
{
    BlockPos vinePos = pos;
    addVine(level, vinePos, property);
    int length = 4;
    vinePos = pos.below();
    while (level->getBlockState(vinePos)->getBlock()->mMaterial == Material::AIR && length > 0)
    {
        addVine(level, vinePos, property);
        vinePos = vinePos.below();
        --length;
    }
}

void TreeFeature::placeCocoa(Level* level, int age, const BlockPos& pos, const Direction* direction)
{
    setBlock(level, pos,
             Blocks::cocoa->defaultBlockState()
                 ->setValue(CocoaBlock::AGE, age)
                 ->setValue(HorizontalDirectionalBlock::FACING_PROPERTY, direction),
             Block::UPDATE_NONE);
}

bool TreeFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "TreeFeature::place");
    int height = random.nextInt(3) + mBaseHeight;
    if (!prepareSpawn(level, pos, height))
    {
        PIXEndNamedEvent();
        return false;
    }
    if (!placeTrunk(level, pos, random, height))
    {
        PIXEndNamedEvent();
        return true;
    }

    PIXBeginNamedEvent(0.0f, "Placing TreeFeature leaves");
    MutableBlockPos leafPos(0, 0, 0);
    for (int y = pos.y + height; y >= pos.y - 3 + height; --y)
    {
        int layer = y - (pos.y + height);
        int radius = 1 - layer / 2;
        for (int x = pos.x - radius; x <= pos.x + radius; ++x)
        {
            int xOffset = x - pos.x;
            for (int z = pos.z - radius; z <= pos.z + radius; ++z)
            {
                if (std::abs(xOffset) == radius && std::abs(z - pos.z) == radius
                    && (random.nextInt(2) == 0 || layer == 0))
                    continue;
                leafPos.set(x, y, z);
                placeLeaf(level, leafPos);
            }
        }
    }
    PIXEndNamedEvent();

    if (mHasVines)
    {
        PIXBeginNamedEvent(0.0f, "TreeFeature adding vines");
        MutableBlockPos vinePos(0, 0, 0);
        for (int y = pos.y - 3 + height; y <= pos.y + height; ++y)
        {
            int radius = 2 - (y - (pos.y + height)) / 2;
            for (int x = pos.x - radius; x <= pos.x + radius; ++x)
            {
                for (int z = pos.z - radius; z <= pos.z + radius; ++z)
                {
                    vinePos.set(x, y, z);
                    if (level->getBlockState(vinePos)->getBlock()->mMaterial == Material::LEAVES)
                    {
                        BlockPos west = vinePos.west();
                        BlockPos east = vinePos.east();
                        BlockPos north = vinePos.north();
                        BlockPos south = vinePos.south();
                        if (random.nextInt(4) == 0 && level->getBlockState(west)->getMaterial() == Material::AIR)
                            addHangingVine(level, west, VineBlock::EAST);
                        if (random.nextInt(4) == 0 && level->getBlockState(east)->getMaterial() == Material::AIR)
                            addHangingVine(level, east, VineBlock::WEST);
                        if (random.nextInt(4) == 0 && level->getBlockState(north)->getMaterial() == Material::AIR)
                            addHangingVine(level, north, VineBlock::SOUTH);
                        if (random.nextInt(4) == 0 && level->getBlockState(south)->getMaterial() == Material::AIR)
                            addHangingVine(level, south, VineBlock::NORTH);
                    }
                }
            }
        }
        PIXEndNamedEvent();

        if (random.nextInt(5) == 0 && height > 5)
        {
            PIXBeginNamedEvent(0.0f, "TreeFeature adding cocoa");
            for (int y = 0; y < 2; ++y)
            {
                arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
                for (unsigned int i = 0; i < directions.length; ++i)
                {
                    const Direction* direction = directions.data[i];
                    if (random.nextInt(4 - y) == 0)
                    {
                        const Direction* opposite = direction->getOpposite();
                        BlockPos cocoaPos = pos.offset(opposite->getStepX(), height - 5 + y, opposite->getStepZ());
                        int age = random.nextInt(3);
                        placeCocoa(level, age, cocoaPos, direction);
                    }
                }
            }
            PIXEndNamedEvent();
        }
    }
    PIXEndNamedEvent();
    return true;
}
