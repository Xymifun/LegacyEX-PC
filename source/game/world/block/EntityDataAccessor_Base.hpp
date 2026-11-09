#ifndef GAME_WORLD_BLOCK_ENTITYDATAACCESSOR_BASE_HPP
#define GAME_WORLD_BLOCK_ENTITYDATAACCESSOR_BASE_HPP

class EntityDataAccessor_Base
{
public:
    EntityDataAccessor_Base(int id) : mId(id) {}

    int getId() const { return mId; }

protected:
    int mId;
};

#endif
