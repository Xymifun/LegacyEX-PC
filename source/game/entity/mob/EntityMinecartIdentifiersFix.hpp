#ifndef GAME_ENTITY_MOB_ENTITYMINECARTIDENTIFIERSFIX_HPP
#define GAME_ENTITY_MOB_ENTITYMINECARTIDENTIFIERSFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityMinecartIdentifiersFix : public DataFix
{
public:
    virtual ~EntityMinecartIdentifiersFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
