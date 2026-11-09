#include "game/world/generation/SeagrassFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/SeaGrass.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TypedBoxed.hpp"

SeagrassFeature::SeagrassFeature() : Feature(false), mGenerating(false)
{
}

SeagrassFeature::~SeagrassFeature()
{
}

bool SeagrassFeature::isNotIce(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlock(pos.below());
    if (block != Blocks::ice && block != Blocks::blue_ice && block != Blocks::packed_ice)
        return true;
    return false;
}

bool SeagrassFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (mGenerating && level->isFindingSpawn())
    {
        return false;
    }

    mGenerating = true;
    BlockPos position = origin;
    while (!level->isSolidBlockingBlock(position) && position.y > 0)
    {
        position = position.below();
    }

    if (!level->isSolidBlockingBlock(position))
    {
        return false;
    }

    const BlockState* seagrassState = Blocks::sea_grass->defaultBlockState();
    if (random.nextInt(20) == 0)
    {
        for (int attempt = 0; attempt < 14; ++attempt)
        {
            const int dz = random.nextGaussianInt(8);
            const int dx = random.nextGaussianInt(8);
            BlockPos target = position.offset(dx, 0, dz);
            while (!level->isEmptyWaterBlock(target) && level->getBlockId(target.x, target.y, target.z) != BlockId::AIR)
            {
                target = target.above();
            }

            if (level->getBlockId(target.x, target.y, target.z) != BlockId::AIR && level->isEmptyWaterBlock(target)
                && static_cast<SeaGrass*>(Blocks::sea_grass)->SeaGrass::canSurvive(level, target, seagrassState, true)
                && isNotIce(level, target))
            {
                level->setBlock(target, seagrassState, Block::UPDATE_CLIENTS | Block::UPDATE_NO_OBSERVERS, false);
            }
        }
        mGenerating = false;
        return true;
    }

    if (random.nextInt(20) == 0)
    {
        for (int attempt = 0; attempt < 8; ++attempt)
        {
            const int dz = random.nextGaussianInt(8);
            const int dx = random.nextGaussianInt(8);
            BlockPos target = position.offset(dx, 0, dz);
            while (!level->isEmptyWaterBlock(target) && level->getBlockId(target.x, target.y, target.z) != BlockId::AIR)
            {
                target = target.above();
            }

            if (level->getBlockId(target.x, target.y, target.z) != BlockId::AIR && level->isEmptyWaterBlock(target)
                && static_cast<SeaGrass*>(Blocks::sea_grass)->SeaGrass::canSurvive(level, target, seagrassState, true)
                && isNotIce(level, target))
            {
                if (level->isEmptyWaterBlock(target.above(2)) && level->isEmptyWaterBlock(target.above())
                    && random.nextInt(9) != 0)
                {
                    const BlockState* lowerState = seagrassState->setValue(
                        SeaGrass::TYPE, static_cast<int>(SeaGrass::TYPE_TALL_BOTTOM));
                    const BlockState* upperState = seagrassState->setValue(SeaGrass::TYPE,
                                                                           static_cast<int>(SeaGrass::TYPE_TALL_TOP));
                    level->setBlock(target, lowerState, Block::UPDATE_CLIENTS | Block::UPDATE_NO_OBSERVERS, false);
                    level->setBlock(target.above(), upperState, Block::UPDATE_CLIENTS | Block::UPDATE_NO_OBSERVERS,
                                    false);
                }
                else
                {
                    level->setBlock(target, seagrassState, Block::UPDATE_CLIENTS | Block::UPDATE_NO_OBSERVERS, false);
                }
            }
        }
        mGenerating = false;
        return true;
    }

    for (int attempt = 0; attempt < 8; ++attempt)
    {
        const int dz = random.nextGaussianInt(8);
        const int dx = random.nextGaussianInt(8);
        BlockPos target = position.offset(dx, 0, dz);
        while (!level->isEmptyWaterBlock(target) && level->getBlockId(target.x, target.y, target.z) != BlockId::AIR)
        {
            target = target.above();
        }

        if (level->getBlockId(target.x, target.y, target.z) != BlockId::AIR && level->isEmptyWaterBlock(target)
            && static_cast<SeaGrass*>(Blocks::sea_grass)->SeaGrass::canSurvive(level, target, seagrassState, true))
        {
            level->setBlock(target, seagrassState, Block::UPDATE_CLIENTS | Block::UPDATE_NO_OBSERVERS, false);
        }
    }

    mGenerating = false;
    return true;
}
