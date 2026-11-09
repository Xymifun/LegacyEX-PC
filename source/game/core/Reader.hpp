#ifndef GAME_CORE_READER_HPP
#define GAME_CORE_READER_HPP

class Reader
{
public:
    virtual ~Reader() {}

    virtual void close() = 0;
    virtual int read() = 0;
    virtual unsigned int read(wchar_t* buffer, unsigned int offset, unsigned int length) = 0;
};

#endif
