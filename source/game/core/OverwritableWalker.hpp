#ifndef GAME_CORE_OVERWRITABLEWALKER_HPP
#define GAME_CORE_OVERWRITABLEWALKER_HPP

#include <string>

#include "game/core/DataWalker.hpp"

class OverwritableWalker : public DataWalker
{
public:
    OverwritableWalker(unsigned long long type, const std::wstring& name);
    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);

private:
    unsigned long long mType;
    std::wstring mName;
};

#endif
