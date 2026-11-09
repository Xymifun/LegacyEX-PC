#ifndef GAME_CORE_BUFFEREDREADER_HPP
#define GAME_CORE_BUFFEREDREADER_HPP

#include <string>

#include "game/core/Reader.hpp"

class BufferedReader : public Reader
{
public:
    BufferedReader(Reader* reader);
    virtual ~BufferedReader();

    virtual void close();
    virtual int read();
    virtual unsigned int read(wchar_t* buffer, unsigned int offset, unsigned int length);

    std::wstring readLine();

private:
    void bufferMore();

    Reader* mReader;
    wchar_t* mBuffer;
    unsigned int mPosition;
    unsigned int mCount;
    unsigned int mCapacity;
};

#endif
