#include "game/world/generation/FlowerFeature.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

FlowerFeature::FlowerFeature(FlowerBlock* block, FlowerBlock::Flower* flower) : Feature(false)
{
    setNextFlower(block, flower);
}

FlowerFeature::~FlowerFeature()
{
}

void FlowerFeature::setNextFlower(FlowerBlock* block, FlowerBlock::Flower* flower)
{
    mFlowerBlock = block;
    const BlockState* base = block->defaultBlockState();
    const Property* property = block->getFlowerProperty();
    const BlockState* state;
    {
        TypedBoxed<FlowerBlock::Flower*> value(flower);
        state = base->setBoxedValue(property, &value);
    }
    mState = state;
}

bool FlowerFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (gApp.getLevelGenerationOptions() != NULL)
    {
        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        BlockPos minPos;
        BlockPos maxPos;
        minPos = origin.offset(-8, -4, -8);
        maxPos = origin.offset(8, 4, 8);
        if (options->checkIntersects(minPos, maxPos))
        {
            return false;
        }
    }

    for (int attempt = 0; attempt < 64; ++attempt)
    {
        const int xOffsetStart = random.nextInt(8);
        const int dx = xOffsetStart - random.nextInt(8);
        const int yOffsetStart = random.nextInt(4);
        const int dy = yOffsetStart - random.nextInt(4);
        const int zOffsetStart = random.nextInt(8);
        const int dz = zOffsetStart - random.nextInt(8);
        const BlockPos position = origin.offset(dx, dy, dz);
        if (level->isEmptyBlock(position) && (!level->mDimension->isHasCeiling() || position.y < 127)
            && mFlowerBlock->canSurvive(level, position, mState))
        {
            level->setBlock(position, mState, Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
