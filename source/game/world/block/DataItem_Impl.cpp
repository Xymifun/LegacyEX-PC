#include "game/world/block/DataItem_Impl.hpp"

#include "game/core/CriticalSection.hpp"
#include "game/core/Rotations.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/EntityDataAccessor.hpp"

DataItem_Impl<BlockPos*>::~DataItem_Impl()
{
    BlockPos* old = mValue;
    delete old;
}

DataItem* DataItem_Impl<BlockPos*>::copy() const
{
    return new DataItem_Impl<BlockPos*>(mAccessor, mAccessor->getSerializer()->copy(mValue));
}

EntityDataSerializerEnum::EEntityDataSerializer DataItem_Impl<BlockPos*>::getSerializerId()
{
    return mAccessor->getSerializer()->getId();
}

int DataItem_Impl<BlockPos*>::getAccessorId()
{
    return mAccessor->getId();
}

void DataItem_Impl<BlockPos*>::setValue(BlockPos* value)
{
    BlockPos* old = mValue;
    delete old;
    mValue = value;
}

bool DataItem_Impl<BlockPos*>::shouldUpdate(BlockPos* value)
{
    if (mValue == NULL || value == NULL)
    {
        return mValue != value;
    }

    bool changed = mValue->x != value->z || mValue->y != value->y || mValue->z != value->z;
    return changed;
}

DataItem_Impl<CompoundTag*>::~DataItem_Impl()
{
    delete mValue;
}

DataItem* DataItem_Impl<CompoundTag*>::copy() const
{
    return new DataItem_Impl<CompoundTag*>(mAccessor, mAccessor->getSerializer()->copy(mValue));
}

EntityDataSerializerEnum::EEntityDataSerializer DataItem_Impl<CompoundTag*>::getSerializerId()
{
    return mAccessor->getSerializer()->getId();
}

int DataItem_Impl<CompoundTag*>::getAccessorId()
{
    return mAccessor->getId();
}

void DataItem_Impl<CompoundTag*>::setValue(CompoundTag* value)
{
    EnterCriticalSection(&SynchedEntityData::sDataItemLock);

    delete mValue;
    mValue = value;

    LeaveCriticalSection(&SynchedEntityData::sDataItemLock);
}

DataItem_Impl<const Rotations*>::DataItem_Impl(const EntityDataAccessor<const Rotations*>* accessor,
                                               const Rotations* value)
{
    mAccessor = accessor;
    mValue = value;
}

DataItem_Impl<const Rotations*>::~DataItem_Impl()
{
    delete mValue;
}

void DataItem_Impl<const Rotations*>::setValue(const Rotations* value)
{
    EnterCriticalSection(&SynchedEntityData::sDataItemLock);

    delete mValue;

    if (value != NULL)
    {
        mValue = new Rotations(*value);
        LeaveCriticalSection(&SynchedEntityData::sDataItemLock);
    }
    else
    {
        mValue = NULL;
        LeaveCriticalSection(&SynchedEntityData::sDataItemLock);
    }
}

const EntityDataAccessor<const Rotations*>* DataItem_Impl<const Rotations*>::getAccessor()
{
    return mAccessor;
}

const Rotations* DataItem_Impl<const Rotations*>::getValue()
{
    return mValue;
}

bool DataItem_Impl<const Rotations*>::shouldUpdate(const Rotations* value)
{
    if (mValue == NULL || value == NULL)
    {
        return mValue != value;
    }

    return !mValue->equals(value);
}

DataItem* DataItem_Impl<const Rotations*>::copy() const
{
    return new DataItem_Impl<const Rotations*>(mAccessor, mAccessor->getSerializer()->copy(mValue));
}

EntityDataSerializerEnum::EEntityDataSerializer DataItem_Impl<const Rotations*>::getSerializerId()
{
    return mAccessor->getSerializer()->getId();
}

int DataItem_Impl<const Rotations*>::getAccessorId()
{
    return mAccessor->getId();
}
