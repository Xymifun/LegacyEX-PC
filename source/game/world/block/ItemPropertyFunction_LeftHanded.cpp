#include "game/world/block/ItemPropertyFunction_LeftHanded.hpp"

#include "game/core/HumanoidArm.hpp"
#include "game/entity/mob/LivingEntity.hpp"

float ItemPropertyFunction_LeftHanded::call(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                            boost::shared_ptr<LivingEntity> entity)
{
    if (!entity || entity->getMainArm() == HumanoidArm::RIGHT)
    {
        return 0.0f;
    }

    return 1.0f;
}
