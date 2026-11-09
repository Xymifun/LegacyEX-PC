#ifndef GAME_ENTITY_MOB_OWNABLEENTITY_HPP
#define GAME_ENTITY_MOB_OWNABLEENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

class Entity;

class OwnableEntity
{
public:
    virtual ~OwnableEntity() {}

    virtual std::wstring getOwnerUUID() = 0;
    virtual boost::shared_ptr<Entity> getOwner() = 0;
};

#endif
