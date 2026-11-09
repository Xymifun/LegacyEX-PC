#include "game/entity/ai/LivingEntitySelector.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

bool LivingEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
    {
        return boost::static_pointer_cast<LivingEntity>(entity)->getMobType() != MOB_TYPE_UNDEAD
               && boost::static_pointer_cast<LivingEntity>(entity)->attackable();
    }
    return false;
}

LivingEntitySelector::~LivingEntitySelector()
{
}
