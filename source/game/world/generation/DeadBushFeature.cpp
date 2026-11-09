#include "game/world/generation/DeadBushFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

DeadBushFeature::DeadBushFeature(Block* block) : Feature(false)
{
}

DeadBushFeature::~DeadBushFeature()
{
}

bool DeadBushFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos pos = origin;
    for (const BlockState* state = level->getBlockState(pos);
         (state->getMaterial() == Material::AIR || state->getMaterial() == Material::LEAVES) && pos.y > 0;
         state = level->getBlockState(pos))
    {
        pos = pos.below();
    }

    for (int i = 0; i < 4; ++i)
    {
        BlockPos target = pos.offset(random.nextInt(8) - random.nextInt(8), random.nextInt(4) - random.nextInt(4),
                                     random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(target)
            && Blocks::deadbush->canSurvive(level, target, Blocks::deadbush->defaultBlockState()))
        {
            level->setBlock(target, Blocks::deadbush->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        }
    }

    return true;
}
