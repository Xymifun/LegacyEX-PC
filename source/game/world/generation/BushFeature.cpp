#include "game/world/generation/BushFeature.hpp"

#include "game/core/Bush.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

BushFeature::BushFeature(Bush* bush) : Feature(false), mBush(bush)
{
}

BushFeature::~BushFeature()
{
}

bool BushFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    for (int attempt = 0; attempt < 64; ++attempt)
    {
        const int dx = random.nextInt(8) - random.nextInt(8);
        const BlockPos position = origin.offset(dx, random.nextInt(4) - random.nextInt(4),
                                                random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(position) && (!level->mDimension->isHasCeiling() || position.y < 127)
            && mBush->canSurvive(level, position, mBush->defaultBlockState()))
        {
            level->setBlock(position, mBush->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
