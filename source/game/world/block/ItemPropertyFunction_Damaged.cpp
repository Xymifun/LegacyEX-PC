#include "game/world/block/ItemPropertyFunction_Damaged.hpp"

#include "game/item/ItemInstance.hpp"

float ItemPropertyFunction_Damaged::call(not_null_ptr<ItemInstance> itemInstance, Level*,
                                         boost::shared_ptr<LivingEntity>)
{
    return itemInstance->isDamaged() ? 1.0f : 0.0f;
}
