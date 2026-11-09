#ifndef GAME_WORLD_BLOCK_DATAITEM_IMPL_HPP
#define GAME_WORLD_BLOCK_DATAITEM_IMPL_HPP

#include "game/item/DataItem.hpp"
#include "game/world/block/EntityDataAccessor.hpp"

class BlockPos;
class CompoundTag;
class Rotations;

template <class T>
class DataItem_Impl : public DataItem
{
public:
    DataItem_Impl(const EntityDataAccessor<T>* accessor, T value)
    {
        mAccessor = accessor;
        mValue = value;
    }
    virtual ~DataItem_Impl();

    virtual DataItem* copy() const;
    virtual EntityDataSerializerEnum::EEntityDataSerializer getSerializerId();
    virtual int getAccessorId();
    virtual void setValue(T value);

    T getValue() { return mValue; }
    const EntityDataAccessor<T>* getAccessor() { return mAccessor; }
    bool shouldUpdate(T value) { return value != getValue(); }

private:
    const EntityDataAccessor<T>* mAccessor;
    T mValue;
};

template <class T>
DataItem_Impl<T>::~DataItem_Impl()
{
}

template <class T>
DataItem* DataItem_Impl<T>::copy() const
{
    return new DataItem_Impl<T>(mAccessor, mAccessor->getSerializer()->copy(mValue));
}

template <class T>
EntityDataSerializerEnum::EEntityDataSerializer DataItem_Impl<T>::getSerializerId()
{
    return mAccessor->getSerializer()->getId();
}

template <class T>
int DataItem_Impl<T>::getAccessorId()
{
    return mAccessor->getId();
}

template <class T>
void DataItem_Impl<T>::setValue(T value)
{
    mValue = value;
}

template <>
class DataItem_Impl<BlockPos*> : public DataItem
{
public:
    DataItem_Impl(const EntityDataAccessor<BlockPos*>* accessor, BlockPos* value)
    {
        mAccessor = accessor;
        mValue = value;
    }
    virtual ~DataItem_Impl();

    virtual DataItem* copy() const;
    virtual EntityDataSerializerEnum::EEntityDataSerializer getSerializerId();
    virtual int getAccessorId();
    virtual void setValue(BlockPos* value);

    BlockPos* getValue() { return mValue; }
    const EntityDataAccessor<BlockPos*>* getAccessor() { return mAccessor; }
    bool shouldUpdate(BlockPos* value);

private:
    const EntityDataAccessor<BlockPos*>* mAccessor;
    BlockPos* mValue;
};

template <>
class DataItem_Impl<CompoundTag*> : public DataItem
{
public:
    DataItem_Impl(const EntityDataAccessor<CompoundTag*>* accessor, CompoundTag* value)
    {
        mAccessor = accessor;
        mValue = value;
    }
    virtual ~DataItem_Impl();

    virtual DataItem* copy() const;
    virtual EntityDataSerializerEnum::EEntityDataSerializer getSerializerId();
    virtual int getAccessorId();
    virtual void setValue(CompoundTag* value);

    CompoundTag* getValue() { return mValue; }
    const EntityDataAccessor<CompoundTag*>* getAccessor() { return mAccessor; }
    bool shouldUpdate(CompoundTag* value) { return value != getValue(); }

private:
    const EntityDataAccessor<CompoundTag*>* mAccessor;
    CompoundTag* mValue;
};

template <>
class DataItem_Impl<const Rotations*> : public DataItem
{
public:
    DataItem_Impl(const EntityDataAccessor<const Rotations*>* accessor, const Rotations* value);
    virtual ~DataItem_Impl();

    virtual DataItem* copy() const;
    virtual EntityDataSerializerEnum::EEntityDataSerializer getSerializerId();
    virtual int getAccessorId();
    virtual void setValue(const Rotations* value);

    const Rotations* getValue();
    const EntityDataAccessor<const Rotations*>* getAccessor();
    bool shouldUpdate(const Rotations* value);

private:
    const EntityDataAccessor<const Rotations*>* mAccessor;
    const Rotations* mValue;
};

#endif
