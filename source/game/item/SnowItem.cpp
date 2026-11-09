#include "game/item/SnowItem.hpp"
#include "game/util/AABB.hpp"

#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

SnowItem::SnowItem(Block* block) : BlockItem(block)
{
    setMaxDamage(0);
    setStackedByData(true);
}

InteractionResult::EInteractionResult SnowItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                      const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                      const Direction* direction, float hitX, float hitY, float hitZ,
                                                      bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (itemInstance->isEmpty() || !player->mayUseItemAt(pos, direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    BlockPos placePos = pos;
    if (direction != Direction::UP || block != mBlock)
    {
        if (!block->mayReplaceWithPlace(level, pos))
        {
            placePos = pos.relative(direction);
            state = level->getBlockState(placePos);
            block = state->getBlock();
        }
    }

    if (block == mBlock)
    {
        if (state->getValue(SnowLayerBlock::LAYERS) < 8)
        {
            if (!simulate)
            {
                const BlockState* snowState = SnowLayerBlock::buildSnowBlock(level, placePos, 1, 1);
                const AABB* shape = snowState->getClipAABB(level, placePos);
                if (shape != Block::NULL_AABB && level->isUnobstructed(shape->move(placePos))
                    && level->setBlock(placePos, snowState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false))
                {
                    const SoundType* soundType = mBlock->getSoundType();
                    level->playSound(player, placePos, soundType->getPlaceSound(), SoundSource::eSoundSource_Blocks,
                                     (soundType->getVolume() + 1.0f) * 0.5f, soundType->getPitch() * 0.8f, 16.0f);
                    itemInstance->shrink(1);
                    return InteractionResult::eInteractionResult_Success;
                }
            }
        }
        else if (player->mayUseItemAt(pos.above(), direction, itemInstance))
        {
            placePos = placePos.above();
        }
    }

    return BlockItem::useOn(player, level, placePos, hand, direction, hitX, hitY, hitZ, simulate);
}

int SnowItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}
