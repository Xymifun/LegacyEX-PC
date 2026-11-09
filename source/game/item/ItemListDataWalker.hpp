#ifndef GAME_ITEM_ITEMLISTDATAWALKER_HPP
#define GAME_ITEM_ITEMLISTDATAWALKER_HPP

#include <string>
#include <vector>

#include "game/core/KeyedDataWalker.hpp"

class CompoundTag;
class DataFixer;

class ItemListDataWalker : public KeyedDataWalker
{
public:
    ItemListDataWalker(unsigned long long type, const std::wstring& itemName);
    ItemListDataWalker(unsigned long long type, const std::vector<std::wstring>& itemNames);

protected:
    virtual CompoundTag* doWalk(DataFixer* fixer, CompoundTag* tag, int version);

private:
    std::vector<std::wstring> mItemNames;
};

#endif
