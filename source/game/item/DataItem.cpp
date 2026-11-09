#include "game/item/DataItem.hpp"

DataItem::DataItem() : mDirty(true)
{
}

EntityDataSerializerEnum::EEntityDataSerializer DataItem::getSerializerId()
{
    return EntityDataSerializerEnum::NUM_ENTITY_DATA_SERIALIZERS;
}

int DataItem::getAccessorId()
{
    return -1;
}

void DataItem::setDirty(bool dirty)
{
    mDirty = dirty;
}

bool DataItem::isDirty()
{
    return mDirty;
}
