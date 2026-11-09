#ifndef GAME_DATA_TAGUPDATEWALKER_HPP
#define GAME_DATA_TAGUPDATEWALKER_HPP

#include <string>

#include "game/core/DataWalker.hpp"
#include "game/data/DataFixTypes.hpp"

class CompoundTag;
class DataFixer;

class TagUpdateWalker : public DataWalker
{
public:
    TagUpdateWalker(const std::wstring& name, unsigned char tagType, DataFixTypes::EDataFixTypes dataFixType);

    virtual CompoundTag* walk(DataFixer* fixer, CompoundTag* tag, int version);

private:
    std::wstring mName;
    unsigned char mTagType;
    DataFixTypes::EDataFixTypes mDataFixType;
};

#endif
