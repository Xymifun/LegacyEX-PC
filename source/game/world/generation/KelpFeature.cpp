#include "game/world/generation/KelpFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/KelpBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

KelpFeature::KelpFeature() : Feature(false)
{
    for (int age = 0; age < 16; ++age)
    {
        mStates[age] = Blocks::kelp->defaultBlockState()->setValue(KelpBlock::AGE, age);
    }
}

KelpFeature::~KelpFeature()
{
}

bool KelpFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (!level->mIsClientSide)
    {
        BlockPos position(origin);
        while (!level->isSolidBlockingBlock(position)
               && level->getBlockId(position.x, position.y, position.z) != BlockId::KELP && position.y > 0)
        {
            position = position.below();
        }

        if (!level->isSolidBlockingBlock(position))
        {
            return false;
        }

        int attempt;
        for (attempt = 0; attempt < 10; ++attempt)
        {
            const int dz = random.nextGaussianInt(8);
            const int dx = random.nextGaussianInt(8);
            BlockPos candidate = position.offset(dx, 0, dz);
            while (!level->isEmptyWaterBlock(candidate)
                   && level->getBlockId(candidate.x, candidate.y, candidate.z) != BlockId::AIR)
            {
                candidate = candidate.above();
            }

            if (level->getBlockId(candidate.x, candidate.y, candidate.z) != BlockId::AIR)
            {
                if (!level->isEmptyWaterBlock(candidate.below())
                    && level->getBlockId(candidate.x, candidate.y, candidate.z) != BlockId::KELP)
                {
                    int height = 0;
                    while (level->isEmptyWaterBlock(candidate.above(height)) && height < 16)
                    {
                        ++height;
                    }

                    if (height >= 3)
                    {
                        int growth = random.nextInt(3, height);
                        int age = 15 - growth;
                        if (Blocks::kelp->mayPlace(level, candidate))
                        {
                            int y;
                            setBlock(level, candidate, mStates[age],
                                     Block::UPDATE_NO_OBSERVERS | Block::UPDATE_LIGHT_IF_CHANGED);
                            for (y = 1; y < growth; ++y)
                            {
                                if (level->isEmptyWaterBlock(candidate.above(y))
                                    && Blocks::kelp->mayPlace(level, candidate.above(y)))
                                {
                                    const BlockState* state = mStates[age + y];
                                    setBlock(level, candidate.above(y), state,
                                             Block::UPDATE_NO_OBSERVERS | Block::UPDATE_LIGHT_IF_CHANGED);
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool KelpFeature::_canPlace(Level* level, const BlockPos& pos) const
{
    int blockId = level->getBlockId(pos.x, pos.y, pos.z);
    return blockId == BlockId::DIRT || blockId == BlockId::SAND || blockId == BlockId::GRAVEL
           || blockId == BlockId::KELP;
}
