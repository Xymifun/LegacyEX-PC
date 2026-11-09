#ifndef GAME_CORE_INPUTSTREAM_HPP
#define GAME_CORE_INPUTSTREAM_HPP

#include <string>

#include "game/world/biome/arrayWithLength.hpp"

class InputStream
{
public:
    virtual ~InputStream() {}

    virtual int read() = 0;
    virtual int read(arrayWithLength<unsigned char> data) = 0;
    virtual int read(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length) = 0;
    virtual void close() = 0;
    virtual long long skip(long long count) = 0;

    static InputStream* getResourceAsStream(const std::wstring& fileName);
};

#endif
