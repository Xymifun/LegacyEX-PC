#ifndef GAME_ENTITY_MOB_ENTITYSHULKERCOLORFIX_HPP
#define GAME_ENTITY_MOB_ENTITYSHULKERCOLORFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntityShulkerColorFix : public DataFix
{
public:
    virtual ~EntityShulkerColorFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
