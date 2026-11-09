#ifndef GAME_ENTITY_MOB_ENTITYSTRINGUUIDFIX_HPP
#define GAME_ENTITY_MOB_ENTITYSTRINGUUIDFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityStringUuidFix : public DataFix
{
public:
    virtual ~EntityStringUuidFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
