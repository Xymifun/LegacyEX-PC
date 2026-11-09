#ifndef GAME_CORE_DATAWALKER_HPP
#define GAME_CORE_DATAWALKER_HPP

#include <string>

class CompoundTag;
class DataFixer;

class DataWalker
{
public:
    virtual ~DataWalker() {}
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version) = 0;

    bool CheckID(CompoundTag* tag, const std::wstring& id);
};

#endif
