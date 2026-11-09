#ifndef GAME_DATA_DATAFIX_HPP
#define GAME_DATA_DATAFIX_HPP

#include <string>

#include "game/data/CompoundTag.hpp"

class DataFix
{
public:
    virtual ~DataFix() {}
    virtual int getVersion() = 0;
    virtual CompoundTag* fix(CompoundTag* tag) = 0;

protected:
    bool CheckID(CompoundTag* tag, const std::wstring& id);
};

#endif
