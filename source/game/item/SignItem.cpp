#include "game/item/SignItem.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Direction.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Stats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/block/StandingSignBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/WallSignBlock.hpp"
#include "game/world/generation/Player.hpp"

SignItem::SignItem()
{
    mMaxStackSize = 16;
}

InteractionResult::EInteractionResult SignItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                      const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                      const Direction* direction, float hitX, float hitY, float hitZ,
                                                      bool simulate)
{
    BlockPos signPos = pos;
    const BlockState* state = level->getBlockState(signPos);
    bool replaceable = state->getBlock()->mayReplaceWithPlace(level, signPos);

    if (direction == Direction::DOWN)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!state->getMaterial()->isSolid() && !replaceable)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (replaceable && direction != Direction::UP)
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    signPos = pos.relative(direction);

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(signPos, direction, itemInstance) || !Blocks::standing_sign->mayPlace(level, signPos))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (level->mIsClientSide)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    if (!simulate)
    {
        if (replaceable)
        {
            signPos = signPos.below();
        }

        if (direction == Direction::UP)
        {
            int rotation = Mth::absFloor((double)((player->mYRot + 180.0f) * 16.0f / 360.0f) + 0.5) & 15;
            level->setBlock(signPos,
                            Blocks::standing_sign->defaultBlockState()->setValue(StandingSignBlock::ROTATION, rotation),
                            Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        }
        else
        {
            level->setBlock(signPos, Blocks::wall_sign->defaultBlockState()->setValue(WallSignBlock::FACING, direction),
                            Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        }

        itemInstance->shrink(1);

        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(signPos);

        if (blockEntity && eTYPE_DERIVED_FROM(BlockEntityType::SIGN, blockEntity->GetType()))
        {
            if (BlockItem::updateCustomBlockEntityTag(level, player, signPos, itemInstance))
            {
                player->openTextEdit(boost::dynamic_pointer_cast<SignBlockEntity>(blockEntity));
            }
        }

        Block* signBlock = direction == Direction::UP ? Blocks::standing_sign : Blocks::wall_sign;

        player->awardStat(GenericStats::blocksPlaced(signBlock),
                          GenericStats::param_blocksPlaced(signBlock->getBlockState(itemInstance->getAuxValue()), 1));
    }

    return InteractionResult::eInteractionResult_Success;
}
