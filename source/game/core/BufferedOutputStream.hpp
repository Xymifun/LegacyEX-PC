#ifndef GAME_CORE_BUFFEREDOUTPUTSTREAM_HPP
#define GAME_CORE_BUFFEREDOUTPUTSTREAM_HPP

#include "game/core/OutputStream.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BufferedOutputStream : public OutputStream
{
public:
    OutputStream* out;
    arrayWithLength<unsigned char> buf;
    unsigned int count;

    BufferedOutputStream(OutputStream* out, int size);
    virtual ~BufferedOutputStream();

    virtual void write(unsigned int b);
    virtual void write(arrayWithLength<unsigned char> b);
    virtual void write(arrayWithLength<unsigned char> b, unsigned int off, unsigned int len);
    virtual void close();
    virtual void flush();
};

#endif
