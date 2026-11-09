#ifndef GAME_ENTITY_MOB_ENTITYHORSESADDLEFIX_HPP
#define GAME_ENTITY_MOB_ENTITYHORSESADDLEFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityHorseSaddleFix : public DataFix
{
public:
    virtual ~EntityHorseSaddleFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
