#include "game/item/ExperienceBottleItem.hpp"
#include "game/core/StringIds.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/ThrownExpBottle.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

ExperienceBottleItem::ExperienceBottleItem() : Item()
{
}

bool ExperienceBottleItem::isFoil(not_null_ptr<ItemInstance>)
{
    return true;
}

InteractionResultHolder ExperienceBottleItem::use(Level* level, boost::shared_ptr<Player> player,
                                                  InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }

    level->playSound(boost::shared_ptr<Player>(), player->mX, player->mY, player->mZ,
                     SoundEvents::EXPERIENCE_BOTTLE_THROW, SoundSource::eSoundSource_Neutral, 0.5f,
                     0.4f / (random->nextFloat() * 0.4f + 0.8f), 16.0f);

    if (!level->mIsClientSide)
    {
        boost::shared_ptr<ThrownExpBottle> bottle(new ThrownExpBottle(level, player));
        bottle->shootFromRotation(player, player->mXRot, player->mYRot, -20.0f, 0.7f, 1.0f);
        level->addEntity(bottle);
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool ExperienceBottleItem::TestUse(Level*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand)
{
    return true;
}

unsigned int ExperienceBottleItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = IDS_NONE;
    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        tooltip = IDS_TOOLTIPS_THROW;
    }
    return tooltip;
}
