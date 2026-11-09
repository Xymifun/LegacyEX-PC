#include "game/world/block/ItemPropertyFunction_Cooldown.hpp"
#include "game/core/EntityType.hpp"

#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"

float ItemPropertyFunction_Cooldown::call(not_null_ptr<ItemInstance> itemInstance, Level*,
                                          boost::shared_ptr<LivingEntity> entity)
{
    return eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
               ? boost::static_pointer_cast<Player>(entity)->getCooldowns()->getCooldownPercent(itemInstance->getItem(),
                                                                                                0.0f)
               : 0.0f;
}
