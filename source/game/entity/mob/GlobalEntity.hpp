#ifndef GAME_ENTITY_MOB_GLOBALENTITY_HPP
#define GAME_ENTITY_MOB_GLOBALENTITY_HPP

#include "game/entity/mob/Entity.hpp"

class GlobalEntity : public Entity
{
public:
    GlobalEntity(Level* level) : Entity(level, true) {}

    virtual ~GlobalEntity() {}
};

#endif
