#ifndef GAME_WORLD_LEVELDATA_DATAWALKER_HPP
#define GAME_WORLD_LEVELDATA_DATAWALKER_HPP

#include "game/core/DataWalker.hpp"

class CompoundTag;
class DataFixer;

class LEVELDATA_DataWalker : public DataWalker
{
public:
    virtual ~LEVELDATA_DataWalker();
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);
};

#endif
