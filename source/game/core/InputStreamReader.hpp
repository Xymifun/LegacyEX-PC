#ifndef GAME_CORE_INPUTSTREAMREADER_HPP
#define GAME_CORE_INPUTSTREAMREADER_HPP

#include "game/core/Reader.hpp"

class DataInputStream;
class InputStream;

class InputStreamReader : public Reader
{
public:
    InputStreamReader(InputStream* stream);
    virtual ~InputStreamReader() {}

    virtual void close();
    virtual int read();
    virtual unsigned int read(wchar_t* buffer, unsigned int offset, unsigned int length);

private:
    DataInputStream* mStream;
};

#endif
