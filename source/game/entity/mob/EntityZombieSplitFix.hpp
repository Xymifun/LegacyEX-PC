#ifndef GAME_ENTITY_MOB_ENTITYZOMBIESPLITFIX_HPP
#define GAME_ENTITY_MOB_ENTITYZOMBIESPLITFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityZombieSplitFix : public DataFix
{
public:
    virtual ~EntityZombieSplitFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
