#include "game/data/DataFix.hpp"

bool DataFix::CheckID(CompoundTag* tag, const std::wstring& id)
{
    return tag->getString(L"id") == id;
}
