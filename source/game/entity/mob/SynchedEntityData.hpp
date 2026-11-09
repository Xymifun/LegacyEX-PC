#ifndef GAME_ENTITY_MOB_SYNCHEDENTITYDATA_HPP
#define GAME_ENTITY_MOB_SYNCHEDENTITYDATA_HPP

#include <vector>

#include <boost/unordered_map.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/item/DataItem.hpp"
#include "game/world/block/DataItem_Impl.hpp"
#include "game/world/block/EntityDataAccessor.hpp"
#include "game/world/block/EntityDataSerializer.hpp"

class DataInputStream;
class DataOutputStream;
struct OSFastMutex;

void MemSect(int section);

class SynchedEntityData
{
public:
    static const int MAX_ID = 255;

    SynchedEntityData(Entity* entity);
    ~SynchedEntityData();

    template <class T>
    static const EntityDataAccessor<T>* defineId(unsigned long long entityType, unsigned long long parentEntityType,
                                                 EntityDataSerializer<T>* serializer)
    {
        int id = defineIdInternal(entityType, parentEntityType);

        sIdsByEntityType[entityType] = id;

        if (serializer != NULL)
        {
            return serializer->createAccessor(id);
        }

        return NULL;
    }

    template <class T>
    void createDateItem(const EntityDataAccessor<T>* accessor, T value);

    template <class T>
    void define(const EntityDataAccessor<T>* accessor, T value)
    {
        createDateItem(accessor, value);
    }

    template <class T>
    T get(const EntityDataAccessor<T>* accessor)
    {
        T value = static_cast<DataItem_Impl<T>*>(mItemsById[accessor->getId()])->getValue();
        return value;
    }

    template <class T>
    void set(const EntityDataAccessor<T>* accessor, T value, bool updateDirty);
    template <class T>
    void markDirty(const EntityDataAccessor<T>* accessor)
    {
        DataItem* item = mItemsById[accessor->getId()];
        item->mDirty = true;
        mIsDirty = true;
    }

    bool isDirty();
    bool isEmpty();

    int getItemCount()
    {
        int count = 0;

        for (int id = 0; id < MAX_ID; id++)
        {
            if (mItemsById[id] != NULL)
            {
                count++;
            }
        }

        return count;
    }
    void clearDirty();

    std::vector<DataItem*>* packDirty();
    std::vector<DataItem*>* getAll();
    void packAll(DataOutputStream* output);
    void assignValues(std::vector<DataItem*>* items);

    static std::vector<DataItem*>* unpack(DataInputStream* input);
    static void pack(std::vector<DataItem*>* items, DataOutputStream* output);
    static void writeDataItem(DataOutputStream* output, DataItem* item);

    static OSFastMutex sDataItemLock;

private:
    static int defineIdInternal(unsigned long long entityType, unsigned long long parentEntityType);

    template <class T>
    static void writeDataItem(DataOutputStream* output, DataItem_Impl<T>* item);

    template <class T>
    void assignValue(DataItem_Impl<T>* target, DataItem_Impl<T>* source);

    static boost::unordered_map<unsigned long long, int> sIdsByEntityType;

    Entity* mEntity;
    DataItem* mItemsById[MAX_ID];
    bool mIsEmpty;
    bool mIsDirty;
};

template <class T>
void SynchedEntityData::createDateItem(const EntityDataAccessor<T>* accessor, T value)
{
    MemSect(17);

    DataItem_Impl<T>* item = new DataItem_Impl<T>(accessor, value);

    if (mItemsById[accessor->getId()] != NULL)
    {
        delete mItemsById[accessor->getId()];
    }

    mIsEmpty = false;
    mItemsById[accessor->getId()] = item;

    MemSect(0);
}

template <class T>
void SynchedEntityData::set(const EntityDataAccessor<T>* accessor, T value, bool updateDirty)
{
    int id = accessor->getId();
    DataItem_Impl<T>* item = static_cast<DataItem_Impl<T>*>(mItemsById[id]);

    if (item->shouldUpdate(value))
    {
        item->setValue(value);
        mEntity->onSyncedDataUpdated(accessor);

        if (updateDirty)
        {
            item->setDirty(true);
            mIsDirty = true;
        }
    }
}

#endif
