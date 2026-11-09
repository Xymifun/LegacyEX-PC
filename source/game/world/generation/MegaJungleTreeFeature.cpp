#include "game/world/generation/MegaJungleTreeFeature.hpp"

#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/VineBlock.hpp"

MegaJungleTreeFeature::MegaJungleTreeFeature(bool doBlockNotify, int baseHeight, int extraRandomHeight,
                                             const BlockState* trunkState, const BlockState* leafState)
    : MegaTreeFeature(doBlockNotify, baseHeight, extraRandomHeight, trunkState, leafState)
{
}

MegaJungleTreeFeature::~MegaJungleTreeFeature()
{
}

void MegaJungleTreeFeature::createCrown(Level* level, const BlockPos& pos, int radius)
{
    for (int i = 0; i >= -2; --i)
    {
        placeDoubleTrunkLeaves(level, pos.above(i), radius + 1 - i);
    }
}

void MegaJungleTreeFeature::placeVine(Level* level, Random& random, const BlockPos& pos,
                                      const BooleanProperty* property)
{
    if (random.nextInt(3) > 0 && level->isEmptyBlock(pos))
    {
        setBlock(level, pos, Blocks::vine->defaultBlockState()->setValue<bool>(property, true), Block::UPDATE_NONE);
    }
}

bool MegaJungleTreeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "MegaJungleTreeFeature::place");

    const int height = calcTreeHeigth(random);
    if (!prepareTree(level, random, origin, height))
    {
        PIXEndNamedEvent();
        return false;
    }

    PIXBeginNamedEvent(0.0f, "Create crown");
    createCrown(level, origin.above(height), 2);
    PIXEndNamedEvent();

    int top = origin.y + height - 2 - random.nextInt(4);

    PIXBeginNamedEvent(0.0f, "Place leaves");
    MutableBlockPos leafPos(0, 0, 0);
    while (top > origin.y + height / 2)
    {
        const float angle = random.nextFloat() * 6.2831855f;
        int x = origin.x + (int)(0.5f + Mth::cos(angle) * 4.0f);
        int z = origin.z + (int)(0.5f + Mth::sin(angle) * 4.0f);
        for (int i = 0; i < 5; ++i)
        {
            x = origin.x + (int)(1.5f + Mth::cos(angle) * (float)i);
            z = origin.z + (int)(1.5f + Mth::sin(angle) * (float)i);
            leafPos.set(x, top - 3 + i / 2, z);
            setBlock(level, leafPos, mTrunkState, Block::UPDATE_NONE);
        }

        const int count = random.nextInt(2) + 1;
        const int crownTop = top;
        for (int y = top - count; y <= crownTop; ++y)
        {
            const int dy = y - crownTop;
            leafPos.set(x, y, z);
            placeSingleTrunkLeaves(level, leafPos, 1 - dy);
        }

        top -= random.nextInt(4) + 2;
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Place trunk");
    for (int y = 0; y < height; ++y)
    {
        const BlockPos trunkPos = origin.above(y);
        if (isFree(level->getBlockState(trunkPos)->getBlock()))
        {
            setBlock(level, trunkPos, mTrunkState, Block::UPDATE_NONE);
            if (y > 0)
            {
                placeVine(level, random, trunkPos.west(), static_cast<const BooleanProperty*>(VineBlock::EAST));
                placeVine(level, random, trunkPos.north(), static_cast<const BooleanProperty*>(VineBlock::SOUTH));
            }
        }

        if (y < height - 1)
        {
            const BlockPos eastPos = trunkPos.east();
            if (isFree(level->getBlockState(eastPos)->getBlock()))
            {
                setBlock(level, eastPos, mTrunkState, Block::UPDATE_NONE);
                if (y > 0)
                {
                    placeVine(level, random, eastPos.east(), static_cast<const BooleanProperty*>(VineBlock::WEST));
                    placeVine(level, random, eastPos.north(), static_cast<const BooleanProperty*>(VineBlock::SOUTH));
                }
            }

            const BlockPos southEastPos = trunkPos.south().east();
            if (isFree(level->getBlockState(southEastPos)->getBlock()))
            {
                setBlock(level, southEastPos, mTrunkState, Block::UPDATE_NONE);
                if (y > 0)
                {
                    placeVine(level, random, southEastPos.east(), static_cast<const BooleanProperty*>(VineBlock::WEST));
                    placeVine(level, random, southEastPos.south(),
                              static_cast<const BooleanProperty*>(VineBlock::NORTH));
                }
            }

            const BlockPos southPos = trunkPos.south();
            if (isFree(level->getBlockState(southPos)->getBlock()))
            {
                setBlock(level, southPos, mTrunkState, Block::UPDATE_NONE);
                if (y > 0)
                {
                    placeVine(level, random, southPos.west(), static_cast<const BooleanProperty*>(VineBlock::EAST));
                    placeVine(level, random, southPos.south(), static_cast<const BooleanProperty*>(VineBlock::NORTH));
                }
            }
        }
    }
    PIXEndNamedEvent();

    PIXEndNamedEvent();
    return true;
}
