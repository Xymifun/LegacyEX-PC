#include "game/entity/ai/EntityByTypeSelector.hpp"

#include "game/entity/mob/Entity.hpp"

EntityByTypeSelector::EntityByTypeSelector(unsigned long long entityType) : mEntityType(entityType)
{
}

bool EntityByTypeSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->instanceof(mEntityType);
}

EntityByTypeSelector::~EntityByTypeSelector()
{
}
