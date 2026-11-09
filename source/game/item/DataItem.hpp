#ifndef GAME_ITEM_DATAITEM_HPP
#define GAME_ITEM_DATAITEM_HPP

#include "game/world/block/EntityDataSerializer.hpp"

class DataItem
{
public:
    DataItem();
    virtual ~DataItem() {}

    virtual DataItem* copy() const = 0;
    virtual EntityDataSerializerEnum::EEntityDataSerializer getSerializerId();
    virtual int getAccessorId();

    bool isDirty();
    void setDirty(bool dirty);

private:
    friend class SynchedEntityData;

    bool mDirty;
};

#endif
