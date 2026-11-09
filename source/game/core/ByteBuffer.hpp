#ifndef GAME_CORE_BYTEBUFFER_HPP
#define GAME_CORE_BYTEBUFFER_HPP

#include "game/core/Buffer.hpp"
#include "game/core/FileHeader.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class FloatBuffer;
class IntBuffer;

class ByteBuffer : public Buffer
{
public:
    ByteBuffer(unsigned int capacity);
    ByteBuffer(unsigned int capacity, unsigned char* bytes);
    virtual ~ByteBuffer();

    virtual ByteBuffer* flip();

    static ByteBuffer* allocate(unsigned int capacity);
    static ByteBuffer* allocateDirect(int capacity);
    static ByteBuffer* wrap(arrayWithLength<unsigned char>& bytes);

    void order(ByteOrder byteOrder);

    unsigned int getSize();
    unsigned char* getBuffer();
    arrayWithLength<unsigned char> array();

    unsigned char get(int index);
    short getShort();
    int getInt();
    unsigned int getInt(unsigned int index);
    long long getLong();
    void getShortArray(arrayWithLength<short>& values);

    ByteBuffer* put(int index, unsigned char value);
    ByteBuffer* put(arrayWithLength<unsigned char> bytes);
    ByteBuffer* putShort(short value);
    ByteBuffer* putInt(int value);
    void putInt(unsigned int index, int value);
    ByteBuffer* putLong(long long value);
    void putShortArray(arrayWithLength<short>& values);

    IntBuffer* asIntBuffer();
    FloatBuffer* asFloatBuffer();

protected:
    unsigned char* mBuffer;
    ByteOrder mOrder;
};

#endif
