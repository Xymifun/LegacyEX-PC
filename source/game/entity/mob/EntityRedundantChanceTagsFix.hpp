#ifndef GAME_ENTITY_MOB_ENTITYREDUNDANTCHANCETAGSFIX_HPP
#define GAME_ENTITY_MOB_ENTITYREDUNDANTCHANCETAGSFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityRedundantChanceTagsFix : public DataFix
{
public:
    virtual ~EntityRedundantChanceTagsFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
