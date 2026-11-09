#ifndef GAME_WORLD_CHUNK_ARRAY2DWITHLENGTH_HPP
#define GAME_WORLD_CHUNK_ARRAY2DWITHLENGTH_HPP

#include "game/world/biome/arrayWithLength.hpp"

template <class T>
class array2DWithLength
{
public:
    arrayWithLength<T>** data;
    unsigned int length;

    array2DWithLength()
    {
        data = NULL;
        length = 0;
    }
    array2DWithLength(unsigned int width, unsigned int height);
    arrayWithLength<T>*& operator[](unsigned int i) { return data[i]; }
};

template <class T>
array2DWithLength<T>::array2DWithLength(unsigned int width, unsigned int height)
{
    data = new arrayWithLength<T>*[width];
    length = width;

    for (unsigned int i = 0; i < length; i++)
    {
        data[i] = new arrayWithLength<T>(height, true);
    }
}

#endif
