#include "game/item/EnderEyeItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EyeOfEnderSignal.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/EndPortalFrameBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"


EnderEyeItem::EnderEyeItem() : Item()
{
}

InteractionResult::EInteractionResult EnderEyeItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                          const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                          const Direction* direction, float hitX, float hitY,
                                                          float hitZ, bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance)
        || state->getBlock() != Blocks::end_portal_frame || state->getValue(EndPortalFrameBlock::EYE))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (simulate)
    {
        return InteractionResult::eInteractionResult_Success;
    }

    level->setBlock(pos, state->setValue(EndPortalFrameBlock::EYE, true), Block::UPDATE_CLIENTS, false);
    level->updateNeighbourForOutputSignal(pos, Blocks::end_portal_frame);
    itemInstance->shrink(1);

    for (int i = 0; i < 16; ++i)
    {
        double px = (float)pos.x + (random->nextFloat() * 6.0f + 5.0f) * 0.0625f;
        double py = (float)pos.y + 0.8125f;
        double pz = (float)pos.z + (random->nextFloat() * 6.0f + 5.0f) * 0.0625f;
        level->addParticle(PARTICLE_SMOKE, px, py, pz, 0.0, 0.0, 0.0, arrayWithLength<int>());
    }
    level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::END_PORTAL_FRAME_FILL,
                     SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, 16.0f);

    BlockPattern* pattern = EndPortalFrameBlock::getOrCreatePortalShape();
    BlockPattern::BlockPatternMatch* match = pattern->find(level, pos);
    if (match != NULL)
    {
        const BlockPos origin = match->getFrontTopLeft().offset(-3, 0, -3);
        for (int x = 0; x < 3; ++x)
        {
            for (int z = 0; z < 3; ++z)
            {
                level->setBlock(origin.offset(x, 0, z), Blocks::end_portal->defaultBlockState(), Block::UPDATE_CLIENTS,
                                false);
            }
        }
        level->globalLevelEvent(1038, origin.offset(1, 0, 1), 0);
    }
    return InteractionResult::eInteractionResult_Success;
}

bool EnderEyeItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hit = getPlayerPOVHitResult(level, player, false);
    if (hit != NULL)
    {
        if (hit->mType == HitResult::BLOCK)
        {
            Block* block = level->getBlockState(hit->getBlockPos())->getBlock();
            delete hit;
            if (block == Blocks::end_portal_frame)
            {
                return false;
            }
        }
        else
        {
            delete hit;
        }
    }
    return true;
}

InteractionResultHolder EnderEyeItem::use(Level* level, boost::shared_ptr<Player> player,
                                          InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hit = getPlayerPOVHitResult(level, player, false);
    if (hit != NULL)
    {
        if (hit->mType == HitResult::BLOCK
            && level->getBlockState(hit->getBlockPos())->getBlock() == Blocks::end_portal_frame)
        {
            delete hit;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
        }
        delete hit;
    }

    player->startUsingItem(hand);
    if (!level->mIsClientSide)
    {
        BlockPos* target = static_cast<ServerLevel*>(level)->getChunkSource()->findNearestMapFeature(
            level, LargeFeature::STRONGHOLD, BlockPos(player), false);
        if (target != NULL)
        {
            boost::shared_ptr<EyeOfEnderSignal> signal(
                new EyeOfEnderSignal(level, player->mX, player->mY + player->mBbHeight * 0.5f, player->mZ));
            signal->signalTo(*target);
            level->addEntity(signal);
            level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ,
                             SoundEvents::ENDEREYE_LAUNCH, SoundSource::eSoundSource_Neutral, 0.5f,
                             0.4f / (random->nextFloat() * 0.4f + 0.8f), 16.0f);
            level->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_ENDER_EYE_LAUNCH, BlockPos(player), 0);
            if (!player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
            player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));
            delete target;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
        }
        delete target;
    }
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int EnderEyeItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_pLevel->mDimension->getType() == DimensionType::OVERWORLD
        && tooltipData.m_pLevel->getLevelData()->getHasStronghold())
    {
        return IDS_TOOLTIPS_THROW;
    }
    return IDS_NONE;
}
