#include "game/entity/mob/SynchedEntityData.hpp"

#include <string>

#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/core/Rotations.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

class Direction;
class CompoundTag;

static const int END_OF_DATA_MARKER = 255;

static bool sDataItemLockCreated = false;

OSFastMutex SynchedEntityData::sDataItemLock;
boost::unordered_map<unsigned long long, int> SynchedEntityData::sIdsByEntityType;

SynchedEntityData::SynchedEntityData(Entity* entity) : mEntity(entity)
{
    mIsEmpty = true;
    mIsDirty = false;

    if (!sDataItemLockCreated)
    {
        InitializeCriticalSection(&sDataItemLock);
        sDataItemLockCreated = true;
    }

    for (int i = 0; i < MAX_ID; ++i)
    {
        mItemsById[i] = NULL;
    }
}

SynchedEntityData::~SynchedEntityData()
{
    for (int i = 0; i < MAX_ID; ++i)
    {
        if (mItemsById[i] != NULL)
        {
            delete mItemsById[i];
        }
    }
}

int SynchedEntityData::defineIdInternal(unsigned long long entityType, unsigned long long parentEntityType)
{
    int id;
    boost::unordered_map<unsigned long long, int>::iterator it = sIdsByEntityType.find(entityType);

    if (it == sIdsByEntityType.end())
    {
        id = 0;
        boost::unordered_map<unsigned long long, int>::iterator parent = sIdsByEntityType.find(parentEntityType);

        if (parent != sIdsByEntityType.end())
        {
            id = parent->second + 1;
        }
    }
    else
    {
        id = it->second + 1;
    }

    return id;
}

bool SynchedEntityData::isDirty()
{
    return mIsDirty;
}

bool SynchedEntityData::isEmpty()
{
    return mIsEmpty;
}

void SynchedEntityData::clearDirty()
{
    mIsDirty = false;

    for (int i = 0; i < MAX_ID; ++i)
    {
        DataItem* item = mItemsById[i];

        if (item != NULL && item->isDirty())
        {
            item->setDirty(false);
        }
    }
}

template <class T>
void SynchedEntityData::writeDataItem(DataOutputStream* output, DataItem_Impl<T>* item)
{
    const EntityDataAccessor<T>* accessor = item->getAccessor();

    EntityDataSerializerEnum::EEntityDataSerializer serializerId = accessor->getSerializer()->getId();

    output->writeByte(accessor->getId());
    output->writeByte(serializerId);

    EnterCriticalSection(&sDataItemLock);
    accessor->getSerializer()->write(output, item->getValue());
    LeaveCriticalSection(&sDataItemLock);
}

void SynchedEntityData::writeDataItem(DataOutputStream* output, DataItem* item)
{
    switch (item->getSerializerId())
    {
    case EntityDataSerializerEnum::BYTE:
        writeDataItem(output, static_cast<DataItem_Impl<unsigned char>*>(item));
        break;

    case EntityDataSerializerEnum::INT:
        writeDataItem(output, static_cast<DataItem_Impl<int>*>(item));
        break;

    case EntityDataSerializerEnum::FLOAT:
        writeDataItem(output, static_cast<DataItem_Impl<float>*>(item));
        break;

    case EntityDataSerializerEnum::STRING:
        writeDataItem(output, static_cast<DataItem_Impl<std::wstring>*>(item));
        break;

    case EntityDataSerializerEnum::ITEM_INSTANCE:
        writeDataItem(output, static_cast<DataItem_Impl<not_null_ptr<ItemInstance> >*>(item));
        break;

    case EntityDataSerializerEnum::BLOCK_STATE:
        writeDataItem(output, static_cast<DataItem_Impl<const BlockState*>*>(item));
        break;

    case EntityDataSerializerEnum::BOOLEAN:
        writeDataItem(output, static_cast<DataItem_Impl<bool>*>(item));
        break;

    case EntityDataSerializerEnum::ROTATIONS:
        writeDataItem(output, static_cast<DataItem_Impl<const Rotations*>*>(item));
        break;

    case EntityDataSerializerEnum::BLOCK_POS:
        writeDataItem(output, static_cast<DataItem_Impl<BlockPos>*>(item));
        break;

    case EntityDataSerializerEnum::OPTIONAL_BLOCK_POS:
        writeDataItem(output, static_cast<DataItem_Impl<BlockPos*>*>(item));
        break;

    case EntityDataSerializerEnum::DIRECTION:
        writeDataItem(output, static_cast<DataItem_Impl<const Direction*>*>(item));
        break;

    case EntityDataSerializerEnum::COMPOUND_TAG:
        writeDataItem(output, static_cast<DataItem_Impl<CompoundTag*>*>(item));
        break;

    default:
        break;
    }
}

void SynchedEntityData::pack(std::vector<DataItem*>* items, DataOutputStream* output)
{
    if (items != NULL)
    {
        std::vector<DataItem*>::iterator end = items->end();

        for (std::vector<DataItem*>::iterator it = items->begin(); it != end; ++it)
        {
            writeDataItem(output, *it);
        }
    }

    output->writeByte(END_OF_DATA_MARKER);
}

std::vector<DataItem*>* SynchedEntityData::packDirty()
{
    std::vector<DataItem*>* items = NULL;

    if (mIsDirty)
    {
        for (int i = 0; i < MAX_ID; ++i)
        {
            DataItem* item = mItemsById[i];

            if (item != NULL && item->isDirty())
            {
                item->setDirty(false);

                if (items == NULL)
                {
                    items = new std::vector<DataItem*>();
                }

                items->push_back(item->copy());
            }
        }
    }

    mIsDirty = false;

    return items;
}

void SynchedEntityData::packAll(DataOutputStream* output)
{
    for (int i = 0; i < MAX_ID; ++i)
    {
        if (mItemsById[i] != NULL)
        {
            writeDataItem(output, mItemsById[i]);
        }
    }

    output->writeByte(END_OF_DATA_MARKER);
}

std::vector<DataItem*>* SynchedEntityData::getAll()
{
    std::vector<DataItem*>* items = NULL;

    for (int i = 0; i < MAX_ID; ++i)
    {
        DataItem* item = mItemsById[i];

        if (item != NULL)
        {
            if (items == NULL)
            {
                items = new std::vector<DataItem*>();
            }

            items->push_back(item->copy());
        }
    }

    return items;
}

std::vector<DataItem*>* SynchedEntityData::unpack(DataInputStream* input)
{
    std::vector<DataItem*>* items = NULL;

    for (int id = input->readByte(); id != END_OF_DATA_MARKER; id = input->readByte())
    {
        if (items == NULL)
        {
            items = new std::vector<DataItem*>();
        }

        switch (input->readByte())
        {
        case EntityDataSerializerEnum::BYTE:
            items->push_back(new DataItem_Impl<unsigned char>(EntityDataSerializers::BYTE->createAccessor(id),
                                                              EntityDataSerializers::BYTE->read(input)));
            break;

        case EntityDataSerializerEnum::INT:
            items->push_back(new DataItem_Impl<int>(EntityDataSerializers::INT->createAccessor(id),
                                                    EntityDataSerializers::INT->read(input)));
            break;

        case EntityDataSerializerEnum::FLOAT:
            items->push_back(new DataItem_Impl<float>(EntityDataSerializers::FLOAT->createAccessor(id),
                                                      EntityDataSerializers::FLOAT->read(input)));
            break;

        case EntityDataSerializerEnum::STRING:
            items->push_back(new DataItem_Impl<std::wstring>(EntityDataSerializers::STRING->createAccessor(id),
                                                             EntityDataSerializers::STRING->read(input)));
            break;

        case EntityDataSerializerEnum::ITEM_INSTANCE:
            items->push_back(
                new DataItem_Impl<not_null_ptr<ItemInstance> >(EntityDataSerializers::ITEM_INSTANCE->createAccessor(id),
                                                               EntityDataSerializers::ITEM_INSTANCE->read(input)));
            break;

        case EntityDataSerializerEnum::BLOCK_STATE:
            items->push_back(
                new DataItem_Impl<const BlockState*>(EntityDataSerializers::BLOCK_STATE->createAccessor(id),
                                                     EntityDataSerializers::BLOCK_STATE->read(input)));
            break;

        case EntityDataSerializerEnum::BOOLEAN:
            items->push_back(new DataItem_Impl<bool>(EntityDataSerializers::BOOLEAN->createAccessor(id),
                                                     EntityDataSerializers::BOOLEAN->read(input)));
            break;

        case EntityDataSerializerEnum::ROTATIONS:
            items->push_back(new DataItem_Impl<const Rotations*>(EntityDataSerializers::ROTATIONS->createAccessor(id),
                                                                 EntityDataSerializers::ROTATIONS->read(input)));
            break;

        case EntityDataSerializerEnum::BLOCK_POS:
            items->push_back(new DataItem_Impl<BlockPos>(EntityDataSerializers::BLOCK_POS->createAccessor(id),
                                                         EntityDataSerializers::BLOCK_POS->read(input)));
            break;

        case EntityDataSerializerEnum::OPTIONAL_BLOCK_POS:
            items->push_back(new DataItem_Impl<BlockPos*>(EntityDataSerializers::OPTIONAL_BLOCK_POS->createAccessor(id),
                                                          EntityDataSerializers::OPTIONAL_BLOCK_POS->read(input)));
            break;

        case EntityDataSerializerEnum::DIRECTION:
            items->push_back(new DataItem_Impl<const Direction*>(EntityDataSerializers::DIRECTION->createAccessor(id),
                                                                 EntityDataSerializers::DIRECTION->read(input)));
            break;

        case EntityDataSerializerEnum::COMPOUND_TAG:
            items->push_back(new DataItem_Impl<CompoundTag*>(EntityDataSerializers::COMPOUND_TAG->createAccessor(id),
                                                             EntityDataSerializers::COMPOUND_TAG->read(input)));
            break;

        default:
            break;
        }
    }

    return items;
}

template <class T>
void SynchedEntityData::assignValue(DataItem_Impl<T>* target, DataItem_Impl<T>* source)
{
    {
        T value = source->getValue();
        target->setValue(value);
    }
    mEntity->onSyncedDataUpdated(source->getAccessor());
}

template <>
void SynchedEntityData::assignValue(DataItem_Impl<BlockPos*>* target, DataItem_Impl<BlockPos*>* source)
{
    if (source->getValue() == NULL)
    {
        target->setValue(NULL);
    }
    else
    {
        target->setValue(new BlockPos(*source->getValue()));
    }

    mEntity->onSyncedDataUpdated(source->getAccessor());
}

template <>
void SynchedEntityData::assignValue(DataItem_Impl<CompoundTag*>* target, DataItem_Impl<CompoundTag*>* source)
{
    if (source->getValue() == NULL)
    {
        target->setValue(NULL);
    }
    else
    {
        target->setValue(source->getValue()->copy());
    }

    mEntity->onSyncedDataUpdated(source->getAccessor());
}

void SynchedEntityData::assignValues(std::vector<DataItem*>* items)
{
    for (std::vector<DataItem*>::iterator it = items->begin(); it != items->end(); ++it)
    {
        DataItem* source = *it;
        DataItem* target = mItemsById[source->getAccessorId()];

        if (target != NULL)
        {
            switch (source->getSerializerId())
            {
            case EntityDataSerializerEnum::BYTE:
                assignValue(static_cast<DataItem_Impl<unsigned char>*>(target),
                            static_cast<DataItem_Impl<unsigned char>*>(source));
                break;

            case EntityDataSerializerEnum::INT:
                assignValue(static_cast<DataItem_Impl<int>*>(target), static_cast<DataItem_Impl<int>*>(source));
                break;

            case EntityDataSerializerEnum::FLOAT:
                assignValue(static_cast<DataItem_Impl<float>*>(target), static_cast<DataItem_Impl<float>*>(source));
                break;

            case EntityDataSerializerEnum::STRING:
                assignValue(static_cast<DataItem_Impl<std::wstring>*>(target),
                            static_cast<DataItem_Impl<std::wstring>*>(source));
                break;

            case EntityDataSerializerEnum::ITEM_INSTANCE:
                assignValue(static_cast<DataItem_Impl<not_null_ptr<ItemInstance> >*>(target),
                            static_cast<DataItem_Impl<not_null_ptr<ItemInstance> >*>(source));
                break;

            case EntityDataSerializerEnum::BLOCK_STATE:
                assignValue(static_cast<DataItem_Impl<const BlockState*>*>(target),
                            static_cast<DataItem_Impl<const BlockState*>*>(source));
                break;

            case EntityDataSerializerEnum::BOOLEAN:
                assignValue(static_cast<DataItem_Impl<bool>*>(target), static_cast<DataItem_Impl<bool>*>(source));
                break;

            case EntityDataSerializerEnum::ROTATIONS:
                assignValue(static_cast<DataItem_Impl<const Rotations*>*>(target),
                            static_cast<DataItem_Impl<const Rotations*>*>(source));
                break;

            case EntityDataSerializerEnum::BLOCK_POS:
                assignValue(static_cast<DataItem_Impl<BlockPos>*>(target),
                            static_cast<DataItem_Impl<BlockPos>*>(source));
                break;

            case EntityDataSerializerEnum::OPTIONAL_BLOCK_POS:
                assignValue(static_cast<DataItem_Impl<BlockPos*>*>(target),
                            static_cast<DataItem_Impl<BlockPos*>*>(source));
                break;

            case EntityDataSerializerEnum::DIRECTION:
                assignValue(static_cast<DataItem_Impl<const Direction*>*>(target),
                            static_cast<DataItem_Impl<const Direction*>*>(source));
                break;

            case EntityDataSerializerEnum::COMPOUND_TAG:
                assignValue(static_cast<DataItem_Impl<CompoundTag*>*>(target),
                            static_cast<DataItem_Impl<CompoundTag*>*>(source));
                break;

            default:
                break;
            }
        }
    }

    mIsDirty = true;
}

template void SynchedEntityData::createDateItem<not_null_ptr<ItemInstance> >(
    const EntityDataAccessor<not_null_ptr<ItemInstance> >*, not_null_ptr<ItemInstance>);
template void SynchedEntityData::createDateItem<std::wstring>(const EntityDataAccessor<std::wstring>*, std::wstring);
template void SynchedEntityData::createDateItem<BlockPos>(const EntityDataAccessor<BlockPos>*, BlockPos);
template void SynchedEntityData::createDateItem<float>(const EntityDataAccessor<float>*, float);
template void SynchedEntityData::createDateItem<int>(const EntityDataAccessor<int>*, int);
template void SynchedEntityData::createDateItem<bool>(const EntityDataAccessor<bool>*, bool);
template void SynchedEntityData::createDateItem<unsigned char>(const EntityDataAccessor<unsigned char>*, unsigned char);
template void SynchedEntityData::createDateItem<const BlockState*>(const EntityDataAccessor<const BlockState*>*,
                                                                   const BlockState*);
template void SynchedEntityData::createDateItem<BlockPos*>(const EntityDataAccessor<BlockPos*>*, BlockPos*);
template void SynchedEntityData::createDateItem<const Direction*>(const EntityDataAccessor<const Direction*>*,
                                                                  const Direction*);
template void SynchedEntityData::createDateItem<CompoundTag*>(const EntityDataAccessor<CompoundTag*>*, CompoundTag*);

template void SynchedEntityData::set<not_null_ptr<ItemInstance> >(
    const EntityDataAccessor<not_null_ptr<ItemInstance> >*, not_null_ptr<ItemInstance>, bool);
template void SynchedEntityData::set<std::wstring>(const EntityDataAccessor<std::wstring>*, std::wstring, bool);
template void SynchedEntityData::set<BlockPos>(const EntityDataAccessor<BlockPos>*, BlockPos, bool);
template void SynchedEntityData::set<BlockPos*>(const EntityDataAccessor<BlockPos*>*, BlockPos*, bool);
template void SynchedEntityData::set<unsigned char>(const EntityDataAccessor<unsigned char>*, unsigned char, bool);
template void SynchedEntityData::set<bool>(const EntityDataAccessor<bool>*, bool, bool);
template void SynchedEntityData::set<float>(const EntityDataAccessor<float>*, float, bool);
template void SynchedEntityData::set<const BlockState*>(const EntityDataAccessor<const BlockState*>*, const BlockState*,
                                                        bool);
template void SynchedEntityData::set<CompoundTag*>(const EntityDataAccessor<CompoundTag*>*, CompoundTag*, bool);
template void SynchedEntityData::set<const Direction*>(const EntityDataAccessor<const Direction*>*, const Direction*,
                                                       bool);

template void SynchedEntityData::writeDataItem<not_null_ptr<ItemInstance> >(
    DataOutputStream*, DataItem_Impl<not_null_ptr<ItemInstance> >*);
template void SynchedEntityData::writeDataItem<std::wstring>(DataOutputStream*, DataItem_Impl<std::wstring>*);
template void SynchedEntityData::writeDataItem<const Rotations*>(DataOutputStream*, DataItem_Impl<const Rotations*>*);

template void SynchedEntityData::assignValue<not_null_ptr<ItemInstance> >(DataItem_Impl<not_null_ptr<ItemInstance> >*,
                                                                          DataItem_Impl<not_null_ptr<ItemInstance> >*);
template void SynchedEntityData::assignValue<std::wstring>(DataItem_Impl<std::wstring>*, DataItem_Impl<std::wstring>*);
template void SynchedEntityData::assignValue<const Rotations*>(DataItem_Impl<const Rotations*>*,
                                                               DataItem_Impl<const Rotations*>*);
template void SynchedEntityData::assignValue<BlockPos>(DataItem_Impl<BlockPos>*, DataItem_Impl<BlockPos>*);
template void SynchedEntityData::assignValue<unsigned char>(DataItem_Impl<unsigned char>*,
                                                            DataItem_Impl<unsigned char>*);
template void SynchedEntityData::assignValue<int>(DataItem_Impl<int>*, DataItem_Impl<int>*);
template void SynchedEntityData::assignValue<float>(DataItem_Impl<float>*, DataItem_Impl<float>*);
template void SynchedEntityData::assignValue<bool>(DataItem_Impl<bool>*, DataItem_Impl<bool>*);
template void SynchedEntityData::assignValue<const BlockState*>(DataItem_Impl<const BlockState*>*,
                                                                DataItem_Impl<const BlockState*>*);
template void SynchedEntityData::assignValue<const Direction*>(DataItem_Impl<const Direction*>*,
                                                               DataItem_Impl<const Direction*>*);
