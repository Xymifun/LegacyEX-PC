#include "game/entity/ai/PreySelector.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Entity.hpp"

bool PreySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return eTYPE_DERIVED_FROM(EntityType::SHEEP, entity->GetType())
           || eTYPE_DERIVED_FROM(EntityType::RABBIT, entity->GetType());
}

PreySelector::~PreySelector()
{
}
