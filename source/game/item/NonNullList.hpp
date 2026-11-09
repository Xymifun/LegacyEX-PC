#ifndef GAME_ITEM_NONNULLLIST_HPP
#define GAME_ITEM_NONNULLLIST_HPP

#include "game/world/biome/arrayWithLength.hpp"

template <class T>
class NonNullList
{
public:
    NonNullList() {}

    ~NonNullList() {}

    static NonNullList<T> withSize(unsigned int size, const T& defaultValue);

    unsigned int size() const { return items.length; }

    T& operator[](unsigned int index) { return items.data[index]; }

    const T& operator[](unsigned int index) const { return items.data[index]; }

    void set(unsigned int index, const T& value) { items.data[index] = value; }

    void clear()
    {
        for (unsigned int i = 0; i < size(); ++i)
        {
            set(i, defaultValue);
        }
    }

    arrayWithLength<T> items;
    T defaultValue;

private:
    NonNullList(arrayWithLength<T> array, const T& defaultValue) : items(array), defaultValue(defaultValue) {}
};

template <class T>
NonNullList<T> NonNullList<T>::withSize(unsigned int size, const T& defaultValue)
{
    if (size == 0)
    {
        return NonNullList<T>(arrayWithLength<T>(), defaultValue);
    }

    arrayWithLength<T> items(size, false);

    for (unsigned int i = 0; i < size; i++)
    {
        items[i] = defaultValue;
    }

    return NonNullList<T>(items, defaultValue);
}

#endif
