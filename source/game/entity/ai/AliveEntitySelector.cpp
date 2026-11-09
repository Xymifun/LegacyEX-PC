#include "game/entity/ai/AliveEntitySelector.hpp"

#include "game/entity/mob/Entity.hpp"

bool AliveEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->isAlive();
}

AliveEntitySelector::~AliveEntitySelector()
{
}
