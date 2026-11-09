#include "game/item/ItemDataWalker.hpp"

#include "game/data/DataFixers.hpp"

ItemDataWalker::ItemDataWalker(unsigned long long type, const std::wstring& itemName) : KeyedDataWalker(type)
{
    mItemNames.push_back(itemName);
}

CompoundTag* ItemDataWalker::doWalk(DataFixer* fixer, CompoundTag* tag, int version)
{
    for (std::vector<std::wstring>::iterator it = mItemNames.begin(); it != mItemNames.end(); ++it)
    {
        tag = DataFixers::updateItem(fixer, tag, version, *it);
    }

    return tag;
}
