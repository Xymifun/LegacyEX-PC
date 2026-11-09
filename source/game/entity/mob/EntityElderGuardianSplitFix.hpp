#ifndef GAME_ENTITY_MOB_ENTITYELDERGUARDIANSPLITFIX_HPP
#define GAME_ENTITY_MOB_ENTITYELDERGUARDIANSPLITFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityElderGuardianSplitFix : public DataFix
{
public:
    virtual ~EntityElderGuardianSplitFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
