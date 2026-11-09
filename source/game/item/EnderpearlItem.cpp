#include "game/item/EnderpearlItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/ThrownEnderpearl.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

EnderpearlItem::EnderpearlItem() : Item()
{
    mMaxStackSize = 16;
}

InteractionResultHolder EnderpearlItem::use(Level* level, boost::shared_ptr<Player> player,
                                            InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }

    level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ, SoundEvents::ENDERPEARL_THROW,
                     SoundSource::eSoundSource_Neutral, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f), 16.0f);

    if (!level->mIsClientSide)
    {
        boost::shared_ptr<ThrownEnderpearl> enderpearl(new ThrownEnderpearl(level, player));
        enderpearl->shootFromRotation(player, player->mXRot, player->mYRot, 0.0f, 1.5f, 1.0f);
        level->addEntity(enderpearl);
        player->getCooldowns()->addCooldown(this, 20);
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool EnderpearlItem::TestUse(Level*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand)
{
    return true;
}

unsigned int EnderpearlItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = IDS_NONE;
    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        tooltip = IDS_TOOLTIPS_THROW;
    }
    return tooltip;
}
