#include "game/entity/ai/LivingShulkerSelector.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"

#include <boost/shared_ptr.hpp>

bool LivingShulkerSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return eTYPE_DERIVED_FROM(EntityType::SHULKER, entity->GetType()) && entity->isAlive();
}

LivingShulkerSelector::~LivingShulkerSelector()
{
}
