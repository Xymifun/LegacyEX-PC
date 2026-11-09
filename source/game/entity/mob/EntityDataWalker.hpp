#ifndef GAME_ENTITY_MOB_ENTITYDATAWALKER_HPP
#define GAME_ENTITY_MOB_ENTITYDATAWALKER_HPP

#include "game/core/DataWalker.hpp"

class CompoundTag;
class DataFixer;

class EntityDataWalker : public DataWalker
{
public:
    virtual ~EntityDataWalker();

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
