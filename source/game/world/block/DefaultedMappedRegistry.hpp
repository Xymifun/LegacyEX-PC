#ifndef GAME_WORLD_BLOCK_DEFAULTEDMAPPEDREGISTRY_HPP
#define GAME_WORLD_BLOCK_DEFAULTEDMAPPEDREGISTRY_HPP

#include <boost/unordered_map.hpp>

#include <utility>

#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/MappedRegistry.hpp"

class Block;

template <class K, class V>
class DefaultedMappedRegistry : public MappedRegistry<K, V>
{
public:
    typedef typename boost::unordered_map<K, V, boost::hash<K>, std::equal_to<K> >::const_iterator iterator;

    DefaultedMappedRegistry(K defaultKey);

    virtual V get(K key);
    virtual V getRandom(Random* random);
    virtual void registerMapping(int id, K key, V value);
    virtual int getId(V value);
    virtual V byId(int id);

    iterator begin() { return this->keySet().begin(); }

    iterator end() { return this->keySet().end(); }

private:
    K mDefaultKey;
    V mDefaultValue;
};

template <class K, class V>
DefaultedMappedRegistry<K, V>::DefaultedMappedRegistry(K defaultKey) : mDefaultKey(defaultKey)
{
    mDefaultValue = V();
}

template <class K, class V>
V DefaultedMappedRegistry<K, V>::get(K key)
{
    if (MappedRegistry<K, V>::containsKey(key))
    {
        return MappedRegistry<K, V>::get(key);
    }

    return mDefaultValue;
}

template <class K, class V>
V DefaultedMappedRegistry<K, V>::getRandom(Random* random)
{
    V value = MappedRegistry<K, V>::getRandom(random);

    if (value == V())
    {
        return mDefaultValue;
    }

    return value;
}

template <class K, class V>
void DefaultedMappedRegistry<K, V>::registerMapping(int id, K key, V value)
{
    if (mDefaultKey == key)
    {
        mDefaultValue = value;
    }

    MappedRegistry<K, V>::registerMapping(id, key, value);
}

template <class K, class V>
int DefaultedMappedRegistry<K, V>::getId(V value)
{
    int id = MappedRegistry<K, V>::getId(value);

    if (id == -1)
    {
        return MappedRegistry<K, V>::getId(mDefaultValue);
    }

    return id;
}

template <class K, class V>
V DefaultedMappedRegistry<K, V>::byId(int id)
{
    if (!MappedRegistry<K, V>::containsId(id))
    {
        return mDefaultValue;
    }

    return MappedRegistry<K, V>::byId(id);
}

template <>
class DefaultedMappedRegistry<ResourceLocation, Block*> : public MappedRegistry<ResourceLocation, Block*>
{
public:
    enum
    {
#ifdef MINECRAFT_PC
        BLOCK_COUNT = 1000
#else
        BLOCK_COUNT = 319
#endif
    };

    class iterator
    {
    private:
        typedef boost::unordered_map<ResourceLocation, Block*, boost::hash<ResourceLocation>,
                                     std::equal_to<ResourceLocation> >::const_iterator base_iterator;

        base_iterator mIterator;
        mutable std::pair<Block*, ResourceLocation> mValue;

        iterator(base_iterator value) : mIterator(value) {}

        void update() const { mValue = std::make_pair(mIterator->second, mIterator->first); }

        friend class DefaultedMappedRegistry<ResourceLocation, Block*>;

    public:
        iterator() {}

        iterator& operator++()
        {
            ++mIterator;
            return *this;
        }

        bool operator!=(const iterator& other) const { return mIterator != other.mIterator; }

        bool operator==(const iterator& other) const { return mIterator == other.mIterator; }

        std::pair<Block*, ResourceLocation>& operator*() const
        {
            update();
            return mValue;
        }

        std::pair<Block*, ResourceLocation>* operator->() const
        {
            update();
            return &mValue;
        }
    };

    DefaultedMappedRegistry(ResourceLocation defaultKey);

    virtual Block* get(ResourceLocation key);
    virtual void registerMapping(int id, ResourceLocation key, Block* value);
    virtual int getId(Block* value);
    virtual Block* byId(int id);
    virtual bool containsId(int id);

    iterator begin() { return iterator(this->keySet().begin()); }

    iterator end() { return iterator(this->keySet().end()); }

private:
    Block* mBlocksById[BLOCK_COUNT];
};

#endif
