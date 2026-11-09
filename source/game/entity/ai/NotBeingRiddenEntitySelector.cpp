#include "game/entity/ai/NotBeingRiddenEntitySelector.hpp"

#include "game/entity/mob/Entity.hpp"

bool NotBeingRiddenEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->isAlive() && !entity->isPassenger() && !entity->isVehicle();
}

NotBeingRiddenEntitySelector::~NotBeingRiddenEntitySelector()
{
}
