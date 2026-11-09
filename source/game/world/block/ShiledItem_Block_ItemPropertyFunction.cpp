#include "game/world/block/ShiledItem_Block_ItemPropertyFunction.hpp"

#include "game/entity/mob/LivingEntity.hpp"

float ShiledItem_Block_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                  boost::shared_ptr<LivingEntity> entity)
{
    return entity && entity->isUsingItem() && entity->getUseItem() == itemInstance ? 1.0f : 0.0f;
}
