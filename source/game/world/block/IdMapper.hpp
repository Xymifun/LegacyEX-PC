#ifndef GAME_WORLD_BLOCK_IDMAPPER_HPP
#define GAME_WORLD_BLOCK_IDMAPPER_HPP

#include <vector>

#include <boost/unordered_map.hpp>

#include "game/world/block/IdMap.hpp"

class BlockState;

template <class T> class IdMapper : public IdMap<T>
{
public:
    IdMapper() {}

    virtual int getId(T value)
    {
        typename boost::unordered_map<T, int>::iterator found = mIds.find(value);
        if (found == mIds.end())
            return -1;
        return found->second;
    }

    virtual T byId(int id)
    {
        if (id >= 0 && id < (int)mObjects.size())
        {
            return mObjects.at(id);
        }

        return T();
    }

    void addMapping(T value, int id);

private:
    boost::unordered_map<T, int> mIds;
    std::vector<T> mObjects;
};

template <class T> void IdMapper<T>::addMapping(T value, int id)
{
    mIds[value] = id;

    while ((int)mObjects.size() <= id)
    {
        mObjects.push_back(T());
    }

    mObjects[id] = value;
}

template <> class IdMapper<const BlockState*> : public IdMap<const BlockState*>
{
public:
    enum
    {
#ifdef MINECRAFT_PC
        BLOCK_STATE_COUNT = 16000
#else
        BLOCK_STATE_COUNT = 5104
#endif
    };

    IdMapper();

    virtual int getId(const BlockState* state);
    virtual const BlockState* byId(int id);
    void addMapping(BlockState* state, int id);
    bool contains(int id);

private:
    const BlockState* mBlockStates[BLOCK_STATE_COUNT];
};

#endif
