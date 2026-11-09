#ifndef GAME_DATA_SAVEDDATA_HPP
#define GAME_DATA_SAVEDDATA_HPP

#include <string>

#include <boost/enable_shared_from_this.hpp>

class CompoundTag;

class SavedData : public boost::enable_shared_from_this<SavedData>
{
public:
    SavedData(const std::wstring& id);
    virtual ~SavedData() {}

    virtual void load(CompoundTag* tag) = 0;
    virtual CompoundTag* save(CompoundTag* tag) = 0;

    void setDirty(bool dirty);
    void setDirty();
    bool isDirty();

    std::wstring id;

private:
    bool dirty;
};

#endif
