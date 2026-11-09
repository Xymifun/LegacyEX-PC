#include "game/core/DataWalker.hpp"

#include "game/data/CompoundTag.hpp"

bool DataWalker::CheckID(CompoundTag* tag, const std::wstring& id)
{
    std::wstring key(L"id");
    std::wstring value = tag->getString(key);
    return value == id;
}
