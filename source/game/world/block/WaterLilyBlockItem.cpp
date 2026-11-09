#include "game/world/block/WaterLilyBlockItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/HitResult.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

WaterLilyBlockItem::WaterLilyBlockItem(Block* block) : TintedBlockItem(block, false)
{
}

bool WaterLilyBlockItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hitResult = Item::getPlayerPOVHitResult(level, player, true);
    if (hitResult == NULL)
    {
        return false;
    }

    if (hitResult->mType == HitResult::BLOCK)
    {
        BlockPos pos = hitResult->getBlockPos();
        if (!level->mayInteract(player, pos, mBlock))
        {
            delete hitResult;
            return false;
        }
        if (!player->mayUseItemAt(pos.relative(hitResult->mDirection), hitResult->mDirection, itemInstance))
        {
            delete hitResult;
            return false;
        }
        delete hitResult;

        BlockPos above = pos.above();
        const BlockState* state = level->getBlockState(pos);
        Block* block = state->getBlock();
        if (block->mMaterial == Material::WATER)
        {
            if (state->getValue(LiquidBlock::LEVEL) == 0 && level->isEmptyBlock(above))
            {
                return true;
            }
        }
    }
    else
    {
        delete hitResult;
    }
    return false;
}

InteractionResultHolder WaterLilyBlockItem::use(Level* level, boost::shared_ptr<Player> player,
                                                InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hitResult = Item::getPlayerPOVHitResult(level, player, true);
    if (hitResult == NULL)
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    if (hitResult->mType == HitResult::BLOCK)
    {
        BlockPos pos = hitResult->getBlockPos();
        if (!level->mayInteract(player, pos, mBlock)
            || !player->mayUseItemAt(pos.relative(hitResult->mDirection), hitResult->mDirection, itemInstance))
        {
            delete hitResult;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
        }
        delete hitResult;

        BlockPos above = pos.above();
        const BlockState* state = level->getBlockState(pos);
        Block* block = state->getBlock();
        if (block->mMaterial == Material::WATER && state->getValue(LiquidBlock::LEVEL) == 0
            && level->isEmptyBlock(above))
        {
            level->setBlock(above, Blocks::waterlily->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE,
                            false);
            if (!player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
            level->playSound(player, pos, SoundEvents::WATERLILY_PLACE, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f,
                             16.0f);
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
        }
    }
    else
    {
        delete hitResult;
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
}

unsigned int WaterLilyBlockItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (TestUse(tooltipData.m_pLevel, tooltipData.m_pPlayer, InteractionHand::eInteractionHand_MainHand))
    {
        return IDS_TOOLTIPS_PLACE;
    }
    return IDS_NONE;
}
