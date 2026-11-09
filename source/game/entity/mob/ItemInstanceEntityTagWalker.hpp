#ifndef GAME_ENTITY_MOB_ITEMINSTANCEENTITYTAGWALKER_HPP
#define GAME_ENTITY_MOB_ITEMINSTANCEENTITYTAGWALKER_HPP

#include "game/core/DataWalker.hpp"

class CompoundTag;
class DataFixer;

class ItemInstanceEntityTagWalker : public DataWalker
{
public:
    virtual ~ItemInstanceEntityTagWalker();

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
