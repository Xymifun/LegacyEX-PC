#ifndef GAME_CORE_OUTPUTSTREAM_HPP
#define GAME_CORE_OUTPUTSTREAM_HPP

#include "game/world/biome/arrayWithLength.hpp"

class OutputStream
{
public:
    virtual ~OutputStream() {}

    virtual void write(unsigned int value) = 0;
    virtual void write(arrayWithLength<unsigned char> data) = 0;
    virtual void write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length) = 0;
    virtual void close() = 0;
    virtual void flush() = 0;
};

#endif
