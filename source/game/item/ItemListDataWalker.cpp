#include "game/item/ItemListDataWalker.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixers.hpp"

ItemListDataWalker::ItemListDataWalker(unsigned long long type, const std::wstring& itemName) : KeyedDataWalker(type)
{
    mItemNames = std::vector<std::wstring>();
    mItemNames.push_back(itemName);
}

ItemListDataWalker::ItemListDataWalker(unsigned long long type, const std::vector<std::wstring>& itemNames)
    : KeyedDataWalker(type)
{
    mItemNames = itemNames;
}

CompoundTag* ItemListDataWalker::doWalk(DataFixer* fixer, CompoundTag* tag, int version)
{
    for (std::vector<std::wstring>::iterator it = mItemNames.begin(); it != mItemNames.end(); ++it)
    {
        tag = DataFixers::updateItems(fixer, tag, version, *it);
    }

    return tag;
}
