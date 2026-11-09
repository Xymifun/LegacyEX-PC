#include "game/world/generation/Feature.hpp"

#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

Feature::Feature(bool doBlockNotify) : mDoBlockNotify(doBlockNotify)
{
}

void Feature::init()
{
}

bool Feature::setBlock(Level* level, const BlockPos& pos, const BlockState* state, int flags)
{
    return level->setBlock(pos, state, flags | (mDoBlockNotify ? 3 : 2), false);
}
