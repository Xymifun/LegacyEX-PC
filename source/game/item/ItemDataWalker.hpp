#ifndef GAME_ITEM_ITEMDATAWALKER_HPP
#define GAME_ITEM_ITEMDATAWALKER_HPP

#include <string>
#include <vector>

#include "game/core/KeyedDataWalker.hpp"

class ItemDataWalker : public KeyedDataWalker
{
public:
    ItemDataWalker(unsigned long long type, const std::wstring& itemName);
    ItemDataWalker(unsigned long long type, const std::vector<std::wstring>& itemNames);

protected:
    virtual CompoundTag* doWalk(DataFixer* fixer, CompoundTag* tag, int version);

private:
    std::vector<std::wstring> mItemNames;
};

#endif
