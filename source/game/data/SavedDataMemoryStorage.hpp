#ifndef GAME_DATA_SAVEDDATAMEMORYSTORAGE_HPP
#define GAME_DATA_SAVEDDATAMEMORYSTORAGE_HPP

#include <string>
#include <typeinfo>

#include <boost/shared_ptr.hpp>

#include "game/data/SavedDataStorage.hpp"

class SavedDataMemoryStorage : public SavedDataStorage
{
public:
    SavedDataMemoryStorage();

    virtual ~SavedDataMemoryStorage();

    virtual boost::shared_ptr<SavedData> get(const std::type_info& type, const std::wstring& id);
    virtual void set(const std::wstring& id, boost::shared_ptr<SavedData> data);
    virtual void save();
    virtual int getFreeAuxValueFor(const std::wstring& id);
};

#endif
