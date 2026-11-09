#ifndef GAME_WORLD_BLOCK_ENTITYDATAACCESSOR_HPP
#define GAME_WORLD_BLOCK_ENTITYDATAACCESSOR_HPP

#include "game/world/block/EntityDataAccessor_Base.hpp"

template <class T>
class EntityDataSerializer;

template <class T>
class EntityDataAccessor : public EntityDataAccessor_Base
{
public:
    EntityDataAccessor(int id, EntityDataSerializer<T>* serializer)
        : EntityDataAccessor_Base(id),
          mSerializer(serializer)
    {
    }

    EntityDataSerializer<T>* getSerializer() const { return mSerializer; }

private:
    EntityDataSerializer<T>* mSerializer;
};

#endif
