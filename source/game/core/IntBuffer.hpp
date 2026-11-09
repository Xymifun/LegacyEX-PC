#ifndef GAME_CORE_INTBUFFER_HPP
#define GAME_CORE_INTBUFFER_HPP

#include "game/core/Buffer.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class IntBuffer : public Buffer
{
public:
    IntBuffer(unsigned int capacity);
    IntBuffer(unsigned int capacity, int* data);
    virtual ~IntBuffer();

    virtual IntBuffer* flip();

    unsigned int get(unsigned int index);
    int* getBuffer();
    IntBuffer* put(arrayWithLength<int>* values, unsigned int offset, unsigned int length);
    IntBuffer* put(arrayWithLength<int> values);
    void put(int value);
    void put(int index, int value);

protected:
    int* mBuffer;
};

#endif
