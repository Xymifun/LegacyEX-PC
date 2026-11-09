#ifndef GAME_WORLD_BLOCK_ENTITYDATASERIALIZER_HPP
#define GAME_WORLD_BLOCK_ENTITYDATASERIALIZER_HPP

#include <boost/unordered_map.hpp>

class DataInputStream;
class DataOutputStream;

template <class T> class EntityDataAccessor;

namespace EntityDataSerializerEnum
{
enum EEntityDataSerializer
{
    BYTE,
    INT,
    FLOAT,
    STRING,
    ITEM_INSTANCE,
    BLOCK_STATE,
    BOOLEAN,
    ROTATIONS,
    BLOCK_POS,
    OPTIONAL_BLOCK_POS,
    DIRECTION,
    COMPOUND_TAG,
    NUM_ENTITY_DATA_SERIALIZERS
};
}

template <class T>
class EntityDataSerializer
{
public:
    EntityDataSerializer(EntityDataSerializerEnum::EEntityDataSerializer id) : mId(id) {}

    virtual void write(DataOutputStream* output, T value) const = 0;
    virtual T read(DataInputStream* input) const = 0;
    virtual EntityDataAccessor<T>* createAccessor(int id) = 0;
    virtual T copy(T value) const = 0;
    virtual EntityDataAccessor<T>* getOrCreateStoredAccessor(int id);

    EntityDataSerializerEnum::EEntityDataSerializer getId() const { return mId; }

private:
    boost::unordered_map<int, EntityDataAccessor<T>*> mAccessors;
    EntityDataSerializerEnum::EEntityDataSerializer mId;
};

template <class T>
EntityDataAccessor<T>* EntityDataSerializer<T>::getOrCreateStoredAccessor(int id)
{
    typename boost::unordered_map<int, EntityDataAccessor<T>*>::iterator it = mAccessors.find(id);

    if (it != mAccessors.end())
    {
        return it->second;
    }

    EntityDataAccessor<T>* accessor = new EntityDataAccessor<T>(id, this);
    mAccessors[id] = accessor;

    return accessor;
}

#endif
