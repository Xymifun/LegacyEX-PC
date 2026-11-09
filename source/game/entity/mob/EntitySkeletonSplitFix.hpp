#ifndef GAME_ENTITY_MOB_ENTITYSKELETONSPLITFIX_HPP
#define GAME_ENTITY_MOB_ENTITYSKELETONSPLITFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class EntitySkeletonSplitFix : public DataFix
{
public:
    virtual ~EntitySkeletonSplitFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif
