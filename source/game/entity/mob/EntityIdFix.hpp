#ifndef GAME_ENTITY_MOB_ENTITYIDFIX_HPP
#define GAME_ENTITY_MOB_ENTITYIDFIX_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityIdFix : public DataFix
{
public:
    virtual ~EntityIdFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
