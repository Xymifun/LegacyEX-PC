#include "game/item/DoorItem.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

DoorItem::DoorItem(Block* block) : Item(), mBlock(block)
{
}

void DoorItem::place(Level* level, const BlockPos& pos, const Direction* direction, Block* block, bool hingeRight)
{
    BlockPos clockwise = pos.relative(direction->getClockWise());
    BlockPos counterClockwise = pos.relative(direction->getCounterClockWise());
    const BlockState* clockwiseState = level->getBlockState(clockwise);
    const BlockState* counterClockwiseState = level->getBlockState(counterClockwise);
    const BlockState* clockwiseAboveState = level->getBlockState(clockwise.above());
    const BlockState* counterClockwiseAboveState = level->getBlockState(counterClockwise.above());
    int counterClockwiseSupport =
        counterClockwiseState->getBlock()->isSolidBlockingCubeAndNotSignalSource(counterClockwiseState)
        + counterClockwiseAboveState->getBlock()->isSolidBlockingCubeAndNotSignalSource(counterClockwiseAboveState);
    int clockwiseSupport = clockwiseState->getBlock()->isSolidBlockingCubeAndNotSignalSource(clockwiseState)
                           + clockwiseAboveState->getBlock()->isSolidBlockingCubeAndNotSignalSource(
                               clockwiseAboveState);
    bool counterClockwiseDoor = counterClockwiseState->getBlock() == block
                                || counterClockwiseAboveState->getBlock() == block;
    bool clockwiseDoor = clockwiseState->getBlock() == block || clockwiseAboveState->getBlock() == block;
    if ((counterClockwiseDoor && !clockwiseDoor) || clockwiseSupport > counterClockwiseSupport)
    {
        hingeRight = true;
    }
    else if ((clockwiseDoor && !counterClockwiseDoor) || clockwiseSupport < counterClockwiseSupport)
    {
        hingeRight = false;
    }

    BlockPos above = pos.above();
    bool powered = level->hasNeighborSignal(pos) || level->hasNeighborSignal(above);
    const BlockState* state = block->defaultBlockState()
                                  ->setValue(DoorBlock::FACING, direction)
                                  ->setValue(DoorBlock::HINGE, hingeRight ? DoorBlock::RIGHT : DoorBlock::LEFT)
                                  ->setValue(DoorBlock::POWERED, powered)
                                  ->setValue(DoorBlock::OPEN, powered);
    level->setBlock(pos, state->setValue(DoorBlock::HALF, DoorBlock::LOWER), Block::UPDATE_ALL, false);
    level->setBlock(above, state->setValue(DoorBlock::HALF, DoorBlock::UPPER), Block::UPDATE_ALL, false);
    level->updateNeighborsAt(pos, block, false);
    level->updateNeighborsAt(above, block, false);
}

InteractionResult::EInteractionResult DoorItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                      const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                      const Direction* direction, float hitX, float hitY, float hitZ,
                                                      bool simulate)
{
    if (direction != Direction::UP)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    BlockPos placePos = pos;
    if (!level->getBlockState(placePos)->getBlock()->mayReplaceWithPlace(level, placePos))
    {
        placePos = placePos.relative(direction);
    }

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(placePos, direction, itemInstance) || !mBlock->mayPlace(level, placePos))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (simulate)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    player->awardStat(GenericStats::blocksPlaced(mBlock),
                      GenericStats::param_blocksPlaced(mBlock->getBlockState(itemInstance->getAuxValue()), 1));

    const Direction* facing = Direction::fromYRot((double)player->mYRot);
    int stepX = facing->getStepX();
    int stepZ = facing->getStepZ();
    bool hingeRight = (stepX < 0 && hitZ < 0.5f) || (stepX > 0 && hitZ > 0.5f) || (stepZ < 0 && hitX > 0.5f)
                      || (stepZ > 0 && hitX < 0.5f);

    place(level, placePos, facing, mBlock, hingeRight);

    const SoundType* soundType = mBlock->getSoundType();
    level->playSound(player, placePos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                     (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
    itemInstance->shrink(1);

    return InteractionResult::eInteractionResult_Success;
}
