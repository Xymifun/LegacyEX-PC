#ifndef GAME_CORE_ENTITIESONLY_DATAWALKER_HPP
#define GAME_CORE_ENTITIESONLY_DATAWALKER_HPP

#include "game/core/DataWalker.hpp"

class CompoundTag;
class DataFixer;

class ENTITIESONLY_DataWalker : public DataWalker
{
public:
    virtual ~ENTITIESONLY_DataWalker();
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
