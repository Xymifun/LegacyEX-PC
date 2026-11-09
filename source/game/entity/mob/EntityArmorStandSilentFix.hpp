#ifndef GAME_ENTITY_MOB_ENTITYARMORSTANDSILENTFIX_HPP
#define GAME_ENTITY_MOB_ENTITYARMORSTANDSILENTFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityArmorStandSilentFix : public DataFix
{
public:
    virtual ~EntityArmorStandSilentFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
