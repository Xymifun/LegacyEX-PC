#include "game/entity/ai/RidingSelector.hpp"

#include "game/entity/mob/Entity.hpp"

bool RidingSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return riding ? entity->getVehicle() == vehicle : entity->getVehicle() != vehicle;
}
