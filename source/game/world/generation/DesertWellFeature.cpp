#include "game/world/generation/DesertWellFeature.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStatePredicate.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/SandBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

static Predicate<const BlockState*>* gDesertWellSandPredicate = NULL;

DesertWellFeature::DesertWellFeature() : Feature(false)
{
    mSlabState = Blocks::stone_slab->defaultBlockState()
                     ->setValue(StoneSlabBlock::VARIANT, StoneSlabBlock::Variant::SAND)
                     ->setValue(HalfSlabBlock::TYPE, HalfSlabBlock::BOTTOM);
    mSandstoneState = Blocks::sandstone->defaultBlockState();
    mWaterState = Blocks::flowing_water->defaultBlockState();
}

void DesertWellFeature::staticCtor()
{
    BlockStatePredicate* sandPredicate = BlockStatePredicate::forBlock(Blocks::sand);
    AbstractProperty<SandBlock::Variant*>* type = SandBlock::TYPE;
    Predicate<SandBlock::Variant*>* isSand = new Predicates<SandBlock::Variant*>::EqualToValuePredicate(
        SandBlock::Variant::SAND);
    gDesertWellSandPredicate = sandPredicate->where(
        type, new BlockStatePredicate::PredicateConversion<SandBlock::Variant*>(isSand));
}

bool DesertWellFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos pos(origin);
    while (level->isEmptyBlock(pos) && pos.y > 2)
        pos = pos.below();

    if (!gDesertWellSandPredicate->apply(level->getBlockState(pos)))
        return false;

    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            if (level->isEmptyBlock(pos.offset(x, -1, z)) && level->isEmptyBlock(pos.offset(x, -2, z)))
                return false;
        }
    }

    for (int y = -1; y <= 0; ++y)
    {
        for (int x = -2; x <= 2; ++x)
        {
            for (int z = -2; z <= 2; ++z)
                level->setBlock(pos.offset(x, y, z), mSandstoneState, Block::UPDATE_CLIENTS, false);
        }
    }

    level->setBlock(pos, mWaterState, Block::UPDATE_CLIENTS, false);
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    const unsigned int directionCount = directions.length;
    for (unsigned int i = 0; i < directionCount; ++i)
        level->setBlock(pos.relative(directions.data[i]), mWaterState, Block::UPDATE_CLIENTS, false);

    for (int x = -2; x <= 2; ++x)
    {
        for (int z = -2; z <= 2; ++z)
        {
            if (x == -2 || x == 2 || z == -2 || z == 2)
                level->setBlock(pos.offset(x, 1, z), mSandstoneState, Block::UPDATE_CLIENTS, false);
        }
    }

    level->setBlock(pos.offset(2, 1, 0), mSlabState, Block::UPDATE_CLIENTS, false);
    level->setBlock(pos.offset(-2, 1, 0), mSlabState, Block::UPDATE_CLIENTS, false);
    level->setBlock(pos.offset(0, 1, 2), mSlabState, Block::UPDATE_CLIENTS, false);
    level->setBlock(pos.offset(0, 1, -2), mSlabState, Block::UPDATE_CLIENTS, false);

    for (int x = -1; x <= 1; ++x)
    {
        for (int z = -1; z <= 1; ++z)
        {
            if (x == 0 && z == 0)
                level->setBlock(pos.offset(x, 4, z), mSandstoneState, Block::UPDATE_CLIENTS, false);
            else
                level->setBlock(pos.offset(x, 4, z), mSlabState, Block::UPDATE_CLIENTS, false);
        }
    }

    for (int y = 1; y <= 3; ++y)
    {
        level->setBlock(pos.offset(-1, y, -1), mSandstoneState, Block::UPDATE_CLIENTS, false);
        level->setBlock(pos.offset(-1, y, 1), mSandstoneState, Block::UPDATE_CLIENTS, false);
        level->setBlock(pos.offset(1, y, -1), mSandstoneState, Block::UPDATE_CLIENTS, false);
        level->setBlock(pos.offset(1, y, 1), mSandstoneState, Block::UPDATE_CLIENTS, false);
    }

    return true;
}
