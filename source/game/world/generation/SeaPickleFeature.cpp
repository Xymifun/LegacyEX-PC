#include "game/world/generation/SeaPickleFeature.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/SeaPickle.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

SeaPickleFeature::~SeaPickleFeature()
{
}

bool SeaPickleFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (!level->mIsClientSide)
    {
        BlockPos position = origin;
        while (!level->isSolidBlockingBlock(position)
               && level->getBlockId(position.x, position.y, position.z) != BlockId::SEA_PICKLE && position.y > 0)
        {
            position = position.below();
        }

        if (!level->isSolidBlockingBlock(position))
        {
            return false;
        }

        for (int attempt = 0; attempt < 4; ++attempt)
        {
            const int dz = random.nextInt(6);
            const int dx = random.nextInt(6);
            BlockPos target = position.offset(dx, 0, dz);
            const int pickles = random.nextInt(0, 4);
            while (!level->isEmptyWaterBlock(target) && level->getBlockId(target.x, target.y, target.z) != BlockId::AIR)
            {
                target = target.above();
            }

            if (level->getBlockId(target.x, target.y, target.z) != BlockId::AIR
                && level->getBlockId(target.x, target.y, target.z) != BlockId::SEA_PICKLE
                && Blocks::sea_pickle->canSurvive(level, target, Blocks::sea_pickle->defaultBlockState()))
            {
                const BlockState* state = Blocks::sea_pickle->defaultBlockState()->setValue(SeaPickle::COUNT, pickles);
                setBlock(level, target, state, Block::UPDATE_NONE);
            }
        }
    }

    return true;
}
