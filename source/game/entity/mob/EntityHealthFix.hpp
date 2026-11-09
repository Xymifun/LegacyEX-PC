#ifndef GAME_ENTITY_MOB_ENTITYHEALTHFIX_HPP
#define GAME_ENTITY_MOB_ENTITYHEALTHFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityHealthFix : public DataFix
{
public:
    virtual ~EntityHealthFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
