#include "game/world/generation/HellSpringFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

HellSpringFeature::HellSpringFeature(Block* block, bool insideRock)
    : Feature(false),
      mBlock(block),
      mInsideRock(insideRock)
{
}

HellSpringFeature::~HellSpringFeature()
{
}

bool HellSpringFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->getBlockState(origin.above())->getBlock() != Blocks::netherrack)
    {
        return false;
    }

    if (level->getBlockState(origin)->getBlock()->mMaterial != Material::AIR)
    {
        Block* block = level->getBlockState(origin)->getBlock();
        if (block != Blocks::netherrack)
        {
            return false;
        }
    }

    int rockSides = 0;
    if (level->getBlockState(origin.west())->getBlock() == Blocks::netherrack)
    {
        ++rockSides;
    }
    if (level->getBlockState(origin.east())->getBlock() == Blocks::netherrack)
    {
        ++rockSides;
    }
    if (level->getBlockState(origin.north())->getBlock() == Blocks::netherrack)
    {
        ++rockSides;
    }
    if (level->getBlockState(origin.south())->getBlock() == Blocks::netherrack)
    {
        ++rockSides;
    }
    if (level->getBlockState(origin.below())->getBlock() == Blocks::netherrack)
    {
        ++rockSides;
    }

    int emptySides = 0;
    if (level->isEmptyBlock(origin.west()))
    {
        ++emptySides;
    }
    if (level->isEmptyBlock(origin.east()))
    {
        ++emptySides;
    }
    if (level->isEmptyBlock(origin.north()))
    {
        ++emptySides;
    }
    if (level->isEmptyBlock(origin.south()))
    {
        ++emptySides;
    }
    if (level->isEmptyBlock(origin.below()))
    {
        ++emptySides;
    }

    if ((!mInsideRock && rockSides == 4 && emptySides == 1) || rockSides == 5)
    {
        const BlockState* state = mBlock->defaultBlockState();
        level->setBlock(origin, state, Block::UPDATE_CLIENTS, false);
        level->instaTick(origin, state, random);
    }

    return true;
}
