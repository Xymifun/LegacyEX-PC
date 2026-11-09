#include "game/world/generation/TallgrassFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

TallgrassFeature::TallgrassFeature(TallGrassBlock::Variant* variant) : Feature(false)
{
    mState = Blocks::tallgrass->defaultBlockState()->setValue(TallGrassBlock::VARIANT, variant);
}

TallgrassFeature::~TallgrassFeature()
{
}

bool TallgrassFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos position(origin);

    const BlockState* state = level->getBlockState(position);
    while ((state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES) && position.y > 0)
    {
        position = position.below();
        state = level->getBlockState(position);
    }

    for (int i = 0; i < 128; ++i)
    {
        const int xOffsetStart = random.nextInt(8);
        const int dx = xOffsetStart - random.nextInt(8);
        const int yOffsetStart = random.nextInt(4);
        const int dy = yOffsetStart - random.nextInt(4);
        const int zOffsetStart = random.nextInt(8);
        const int dz = zOffsetStart - random.nextInt(8);
        const BlockPos candidate = position.offset(dx, dy, dz);
        if (level->isEmptyBlock(candidate) && Blocks::tallgrass->canSurvive(level, candidate, mState))
        {
            level->setBlock(candidate, mState, Block::UPDATE_CLIENTS | Block::UPDATE_NO_OBSERVERS, false);
        }
    }

    return true;
}
