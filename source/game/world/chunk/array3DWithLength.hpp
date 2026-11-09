#ifndef GAME_WORLD_CHUNK_ARRAY3DWITHLENGTH_HPP
#define GAME_WORLD_CHUNK_ARRAY3DWITHLENGTH_HPP

#include "game/world/chunk/array2DWithLength.hpp"

template <class T>
class array3DWithLength
{
public:
    array2DWithLength<T>** data;
    unsigned int length;

    array3DWithLength(unsigned int depth, unsigned int width, unsigned int height)
    {
        data = new array2DWithLength<T>*[depth];
        length = depth;

        for (unsigned int i = 0; i < depth; i++)
        {
            data[i] = new array2DWithLength<T>(width, height);
        }
    }
};

#endif
