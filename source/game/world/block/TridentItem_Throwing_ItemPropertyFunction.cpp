#include "game/world/block/TridentItem_Throwing_ItemPropertyFunction.hpp"

#include "game/entity/mob/LivingEntity.hpp"

float TridentItem_Throwing_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance, Level*,
                                                      boost::shared_ptr<LivingEntity> entity)
{
    return (float)(entity && entity->isUsingItem() && entity->getUseItem() == itemInstance);
}
