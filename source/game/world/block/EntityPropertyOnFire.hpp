#ifndef GAME_WORLD_BLOCK_ENTITYPROPERTYONFIRE_HPP
#define GAME_WORLD_BLOCK_ENTITYPROPERTYONFIRE_HPP

#include "game/world/block/EntityProperty.hpp"

class EntityPropertyOnFire : public EntityProperty
{
private:
    bool m_onFire;

public:
    EntityPropertyOnFire(bool onFire);
    virtual bool applies(Random* random, const boost::shared_ptr<Entity>& entity);
};

#endif
