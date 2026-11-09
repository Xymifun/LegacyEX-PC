#include "game/world/block/EntityPropertyOnFire.hpp"

#include "game/entity/mob/Entity.hpp"

EntityPropertyOnFire::EntityPropertyOnFire(bool onFire) : m_onFire(onFire)
{
}

bool EntityPropertyOnFire::applies(Random* random, const boost::shared_ptr<Entity>& entity)
{
    (void)random;
    return entity->isOnFire() == m_onFire;
}
