#ifndef GAME_ENTITY_MOB_ENTITYHORSESPLITFIX_HPP
#define GAME_ENTITY_MOB_ENTITYHORSESPLITFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityHorseSplitFix : public DataFix
{
public:
    virtual ~EntityHorseSplitFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
