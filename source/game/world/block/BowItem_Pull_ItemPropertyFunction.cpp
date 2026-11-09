#include "game/world/block/BowItem_Pull_ItemPropertyFunction.hpp"

#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

float BowItem_Pull_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance, Level*,
                                              boost::shared_ptr<LivingEntity> entity)
{
    if (entity == NULL)
    {
        return 0.0f;
    }

    if (entity->getUseItem()->getItem() != Items::bow)
    {
        return 0.0f;
    }

    return (float)(itemInstance->getUseDuration() - entity->getUseItemRemainingTicks())
           / (float)BowItem::GetMaxBowDuration();
}
