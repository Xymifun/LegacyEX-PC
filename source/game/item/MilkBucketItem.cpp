#include "game/item/MilkBucketItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GenericStats.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

not_null_ptr<ItemInstance> MilkBucketItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                           boost::shared_ptr<LivingEntity> entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
        && !boost::static_pointer_cast<Player>(entity)->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }

    if (!level->mIsClientSide)
    {
        entity->removeAllEffects();
    }

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::static_pointer_cast<Player>(entity)->awardStat(
            gStats->get_itemsUsed(Item::VALIDgetId(this)),
            GenericStats::param_itemsUsed(boost::static_pointer_cast<Player>(entity), itemInstance));
    }

    if (itemInstance->isEmpty())
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::bucket));
    }

    return itemInstance;
}

int MilkBucketItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return 32;
}

int MilkBucketItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return UseAnim_drink;
}

InteractionResultHolder MilkBucketItem::use(Level* level, boost::shared_ptr<Player> player,
                                            InteractionHand::EInteractionHand hand)
{
    player->startUsingItem(hand);
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int MilkBucketItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return IDS_TOOLTIPS_DRINK;
}
