#ifndef GAME_CORE_ANONYMOUSDATAWALKER_HPP
#define GAME_CORE_ANONYMOUSDATAWALKER_HPP

#include "game/core/DataWalker.hpp"

class CompoundTag;
class DataFixer;

class AnonymousDataWalker : public DataWalker
{
public:
    virtual ~AnonymousDataWalker();
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
