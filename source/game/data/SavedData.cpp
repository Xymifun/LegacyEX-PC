#include "game/data/SavedData.hpp"

SavedData::SavedData(const std::wstring& id) : id(id), dirty(false)
{
}

void SavedData::setDirty(bool dirty)
{
    this->dirty = dirty;
}

void SavedData::setDirty()
{
    setDirty(true);
}

bool SavedData::isDirty()
{
    return dirty;
}
