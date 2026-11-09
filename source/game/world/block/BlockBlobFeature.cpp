#include "game/world/block/BlockBlobFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"

BlockBlobFeature::BlockBlobFeature(Block* block, int radius) : Feature(false), mBlock(block), mRadius(radius)
{
}

BlockBlobFeature::~BlockBlobFeature()
{
}

bool BlockBlobFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position = origin;

    while (position.y > 3)
    {
        if (!level->isEmptyBlock(position.below()))
        {
            Block* block = level->getBlockState(position.below())->getBlock();
            if (block == Blocks::grass || block == Blocks::dirt || block == Blocks::stone)
            {
                break;
            }
        }

        position = position.below();
    }

    if (position.y <= 3)
    {
        return false;
    }

    int radiusBase = mRadius;
    for (int layer = 0; radiusBase >= 0 && layer < 3; ++layer)
    {
        int xRadius = radiusBase + random.nextInt(2);
        int yRadius = radiusBase + random.nextInt(2);
        int zRadius = radiusBase + random.nextInt(2);
        float radius = (float)(xRadius + yRadius + zRadius) * 0.333F + 0.5F;

        Iterator<BlockPos*>* positions = BlockPos::between(position.offset(-xRadius, -yRadius, -zRadius),
                                                           position.offset(xRadius, yRadius, zRadius));
        while (positions->hasNext())
        {
            BlockPos candidate = *positions->next();
            if (candidate.distSqr(position) <= (double)(radius * radius))
            {
                level->setBlock(candidate, mBlock->defaultBlockState(), Block::UPDATE_INVISIBLE, false);
            }
        }
        delete positions;

        position = position.offset(random.nextInt(radiusBase * 2 + 2) - (radiusBase + 1), -random.nextInt(2),
                                   random.nextInt(radiusBase * 2 + 2) - (radiusBase + 1));
    }

    return true;
}
