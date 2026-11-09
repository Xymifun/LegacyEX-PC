#ifndef GAME_CORE_BYTEARRAYOUTPUTSTREAM_HPP
#define GAME_CORE_BYTEARRAYOUTPUTSTREAM_HPP

#include "game/core/OutputStream.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ByteArrayOutputStream : public OutputStream
{
public:
    ByteArrayOutputStream();
    ByteArrayOutputStream(unsigned int size);
    virtual ~ByteArrayOutputStream();

    virtual void write(unsigned int value);
    virtual void write(arrayWithLength<unsigned char> data);
    virtual void write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual void close();
    virtual void flush();
    virtual arrayWithLength<unsigned char> toByteArray();

    arrayWithLength<unsigned char> mBuffer;
    int mCount;
};

#endif
