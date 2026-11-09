#include "game/data/SavedDataMemoryStorage.hpp"

SavedDataMemoryStorage::SavedDataMemoryStorage() : SavedDataStorage(0)
{
}

SavedDataMemoryStorage::~SavedDataMemoryStorage()
{
}

boost::shared_ptr<SavedData> SavedDataMemoryStorage::get(const std::type_info& type, const std::wstring& id)
{
    boost::unordered_map<std::wstring, boost::shared_ptr<SavedData> >::iterator it = mSavedData.find(id);

    if (it != mSavedData.end())
    {
        return it->second;
    }

    return boost::shared_ptr<SavedData>();
}

void SavedDataMemoryStorage::set(const std::wstring& id, boost::shared_ptr<SavedData> data)
{
    mSavedData[id] = data;
}

void SavedDataMemoryStorage::save()
{
}

int SavedDataMemoryStorage::getFreeAuxValueFor(const std::wstring& id)
{
    return 0;
}
