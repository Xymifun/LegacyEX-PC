#include "game/entity/ai/ContainerEntitySelector.hpp"

#include "game/item/Container.hpp"
#include "game/entity/mob/Entity.hpp"

bool ContainerEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(entity);

    return container && entity->isAlive();
}

ContainerEntitySelector::~ContainerEntitySelector()
{
}
