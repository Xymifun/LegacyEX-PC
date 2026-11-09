#include "game/world/block/StandingSignBlock.hpp"

#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* StandingSignBlock::ROTATION = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"rotation", 0, 15));

StandingSignBlock::StandingSignBlock() : SignBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(ROTATION, 0));
}

void StandingSignBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                        const BlockPos& fromPos)
{
    if (!level->getBlockState(pos.below())->getBlock()->mMaterial->isSolid())
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }

    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* StandingSignBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(ROTATION, data);
}

int StandingSignBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(ROTATION);
}

const BlockState* StandingSignBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(ROTATION, rotation->rotate(state->getValue(ROTATION), 16));
}

const BlockState* StandingSignBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return state->setValue(ROTATION, mirror->mirror(state->getValue(ROTATION), 16));
}

BlockStateDefinition* StandingSignBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {ROTATION};
    return new BlockStateDefinition(this, properties, 1);
}
